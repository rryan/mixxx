#include "track/track.h"

#include <QDirIterator>
#include <QMutexLocker>
#include <atomic>

#include "engine/engine.h"
#include "track/beatfactory.h"
#include "track/trackref.h"
#include "util/assert.h"
#include "util/color/color.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("Track");

constexpr bool kLogStats = false;

// Count the number of currently existing instances for detecting
// memory leaks.
std::atomic<int> s_numberOfInstances;

SecurityTokenPointer openSecurityToken(
        const TrackFile& trackFile,
        SecurityTokenPointer pSecurityToken = SecurityTokenPointer()) {
    if (pSecurityToken.isNull()) {
        return Sandbox::openSecurityToken(trackFile.asFileInfo(), true);
    } else {
        return pSecurityToken;
    }
}

template<typename T>
inline
bool compareAndSet(T* pField, const T& value) {
    if (*pField != value) {
        *pField = value;
        return true;
    } else {
        return false;
    }
}

inline
mixxx::Bpm getActualBpm(
        mixxx::Bpm bpm,
        BeatsPointer pBeats = BeatsPointer()) {
    // Only use the imported BPM if the beat grid is not valid!
    // Reason: The BPM value in the metadata might be normalized
    // or rounded, e.g. ID3v2 only supports integer values.
    if (pBeats) {
        return mixxx::Bpm(pBeats->getBpm());
    } else {
        return bpm;
    }
}

} // anonymous namespace

Track::Track(
        TrackFile fileInfo,
        SecurityTokenPointer pSecurityToken,
        TrackId trackId)
        : m_qMutex(QMutex::Recursive),
          m_fileInfo(std::move(fileInfo)),
          m_pSecurityToken(openSecurityToken(m_fileInfo, std::move(pSecurityToken))),
          m_record(trackId),
          m_bDirty(false),
          m_bMarkedForMetadataExport(false) {
    if (kLogStats && kLogger.debugEnabled()) {
        long numberOfInstancesBefore = s_numberOfInstances.fetch_add(1);
        kLogger.debug()
                << "Creating instance:"
                << this
                << numberOfInstancesBefore
                << "->"
                << numberOfInstancesBefore + 1;
    }
}

Track::~Track() {
    if (kLogStats && kLogger.debugEnabled()) {
        long numberOfInstancesBefore = s_numberOfInstances.fetch_sub(1);
        kLogger.debug()
                << "Destroying instance:"
                << this
                << numberOfInstancesBefore
                << "->"
                << numberOfInstancesBefore - 1;
    }
}

//static
TrackPointer Track::newTemporary(
        TrackFile fileInfo,
        SecurityTokenPointer pSecurityToken) {
    return std::make_shared<Track>(
            std::move(fileInfo),
            std::move(pSecurityToken));
}

//static
TrackPointer Track::newDummy(
        TrackFile fileInfo,
        TrackId trackId) {
    return std::make_shared<Track>(
            std::move(fileInfo),
            SecurityTokenPointer(),
            trackId);
}

void Track::relocate(
        TrackFile fileInfo,
        SecurityTokenPointer pSecurityToken) {
    QMutexLocker lock(&m_qMutex);
    m_pSecurityToken = openSecurityToken(fileInfo, std::move(pSecurityToken));
    m_fileInfo = std::move(fileInfo);
    // The track does not need to be marked as dirty,
    // because this function will always be called with
    // the updated location from the database.
}

void Track::importMetadata(
        mixxx::TrackMetadata importedMetadata,
        QDateTime metadataSynchronized) {
    // Safe some values that are needed after move assignment and unlocking(see below)
    const auto newBpm = importedMetadata.getTrackInfo().getBpm();
    const auto newKey = importedMetadata.getTrackInfo().getKey();
    const auto newReplayGain = importedMetadata.getTrackInfo().getReplayGain();
#ifdef __EXTRA_METADATA__
    const auto newSeratoTags = importedMetadata.getTrackInfo().getSeratoTags();
#endif // __EXTRA_METADATA__
    {
        // enter locking scope
        QMutexLocker lock(&m_qMutex);

        bool modified = compareAndSet(
                &m_record.refMetadataSynchronized(),
                !metadataSynchronized.isNull());
        bool modifiedReplayGain = false;
        if (m_record.getMetadata() != importedMetadata) {
            modifiedReplayGain =
                    (m_record.getMetadata().getTrackInfo().getReplayGain() != newReplayGain);
            m_record.setMetadata(std::move(importedMetadata));
            // Don't use importedMetadata after move assignment!!
            modified = true;
        }
        if (modified) {
            // explicitly unlock before emitting signals
            markDirtyAndUnlock(&lock);
            if (modifiedReplayGain) {
                emit ReplayGainUpdated(newReplayGain);
            }
        }

#ifdef __EXTRA_METADATA__
        setColor(newSeratoTags.getTrackColor());
        setBpmLocked(newSeratoTags.isBpmLocked());
#endif // __EXTRA_METADATA__

        // implicitly unlocked when leaving scope
    }

    // Need to set BPM after sample rate since beat grid creation depends on
    // knowing the sample rate. Bug #1020438.
    const auto actualBpm = getActualBpm(newBpm, m_pBeats);
    if (actualBpm.hasValue()) {
        setBpm(actualBpm.getValue());
    }

    if (!newKey.isEmpty()
            && KeyUtils::guessKeyFromText(newKey) != mixxx::track::io::key::INVALID) {
        setKeyText(newKey, mixxx::track::io::key::FILE_METADATA);
    }
}

void Track::mergeImportedMetadata(
        const mixxx::TrackMetadata& importedMetadata) {
    QMutexLocker lock(&m_qMutex);
    m_record.mergeImportedMetadata(importedMetadata);
}

void Track::readTrackMetadata(
        mixxx::TrackMetadata* pTrackMetadata,
        bool* pMetadataSynchronized) const {
    DEBUG_ASSERT(pTrackMetadata);
    QMutexLocker lock(&m_qMutex);
    *pTrackMetadata = m_record.getMetadata();
    if (pMetadataSynchronized != nullptr) {
        *pMetadataSynchronized = m_record.getMetadataSynchronized();
    }
}

void Track::readTrackRecord(
        mixxx::TrackRecord* pTrackRecord,
        bool* pDirty) const {
    DEBUG_ASSERT(pTrackRecord);
    QMutexLocker lock(&m_qMutex);
    *pTrackRecord = m_record;
    if (pDirty != nullptr) {
        *pDirty = m_bDirty;
    }
}

QString Track::getCanonicalLocation() const {
    QMutexLocker lock(&m_qMutex);
    return /*mutable*/ m_fileInfo.freshCanonicalLocation(); // non-const
}

mixxx::ReplayGain Track::getReplayGain() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getReplayGain();
}

void Track::setReplayGain(const mixxx::ReplayGain& replayGain) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refReplayGain(), replayGain)) {
        markDirtyAndUnlock(&lock);
        emit ReplayGainUpdated(replayGain);
    }
}

double Track::getBpm() const {
    double bpm = mixxx::Bpm::kValueUndefined;
    QMutexLocker lock(&m_qMutex);
    if (m_pBeats) {
        // BPM from beat grid overrides BPM from metadata
        // Reason: The BPM value in the metadata might be imprecise,
        // e.g. ID3v2 only supports integer values!
        double beatsBpm = m_pBeats->getBpm();
        if (mixxx::Bpm::isValidValue(beatsBpm)) {
            bpm = beatsBpm;
        }
    }
    return bpm;
}

double Track::setBpm(double bpmValue) {
    if (!mixxx::Bpm::isValidValue(bpmValue)) {
        // If the user sets the BPM to an invalid value, we assume
        // they want to clear the beatgrid.
        setBeats(BeatsPointer());
        return bpmValue;
    }

    QMutexLocker lock(&m_qMutex);

    if (!m_pBeats) {
        // No beat grid available -> create and initialize
        double cue = getCuePoint().getPosition();
        BeatsPointer pBeats(BeatFactory::makeBeatGrid(*this, bpmValue, cue));
        setBeatsAndUnlock(&lock, pBeats);
        return bpmValue;
    }

    // Continue with the regular case
    if (m_pBeats->getBpm() != bpmValue) {
        if (kLogger.debugEnabled()) {
            kLogger.debug() << "Updating BPM:" << getLocation();
        }
        m_pBeats->setBpm(bpmValue);
        markDirtyAndUnlock(&lock);
        // Tell the GUI to update the bpm label...
        //qDebug() << "Track signaling BPM update to" << f;
        emit bpmUpdated(bpmValue);
    }

    return bpmValue;
}

QString Track::getBpmText() const {
    return QString("%1").arg(getBpm(), 3,'f',1);
}

void Track::setBeats(BeatsPointer pBeats) {
    QMutexLocker lock(&m_qMutex);
    setBeatsAndUnlock(&lock, pBeats);
}

void Track::setBeatsAndUnlock(QMutexLocker* pLock, BeatsPointer pBeats) {
    // This whole method is not so great. The fact that Beats is an ABC is
    // limiting with respect to QObject and signals/slots.

    if (m_pBeats == pBeats) {
        pLock->unlock();
        return;
    }

    if (m_pBeats) {
        disconnect(m_pBeats.data(), &Beats::updated, this, &Track::slotBeatsUpdated);
    }

    m_pBeats = std::move(pBeats);

    auto bpmValue = mixxx::Bpm::kValueUndefined;
    if (m_pBeats) {
        bpmValue = m_pBeats->getBpm();
        connect(m_pBeats.data(), &Beats::updated, this, &Track::slotBeatsUpdated);
    }
    m_record.refMetadata().refTrackInfo().setBpm(mixxx::Bpm(bpmValue));

    markDirtyAndUnlock(pLock);
    emit bpmUpdated(bpmValue);
    emit beatsUpdated();
}

BeatsPointer Track::getBeats() const {
    QMutexLocker lock(&m_qMutex);
    return m_pBeats;
}

void Track::slotBeatsUpdated() {
    QMutexLocker lock(&m_qMutex);

    auto bpmValue = mixxx::Bpm::kValueUndefined;
    if (m_pBeats) {
        bpmValue = m_pBeats->getBpm();
    }
    m_record.refMetadata().refTrackInfo().setBpm(mixxx::Bpm(bpmValue));

    markDirtyAndUnlock(&lock);
    emit bpmUpdated(bpmValue);
    emit beatsUpdated();
}

void Track::setMetadataSynchronized(bool metadataSynchronized) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadataSynchronized(), metadataSynchronized)) {
        markDirtyAndUnlock(&lock);
    }
}

bool Track::isMetadataSynchronized() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadataSynchronized();
}

QString Track::getInfo() const {
    QMutexLocker lock(&m_qMutex);
    if (m_record.getMetadata().getTrackInfo().getArtist().trimmed().isEmpty()) {
        return m_record.getMetadata().getTrackInfo().getTitle();
    } else {
        return m_record.getMetadata().getTrackInfo().getArtist() + ", " + m_record.getMetadata().getTrackInfo().getTitle();
    }
}

QDateTime Track::getDateAdded() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getDateAdded();
}

void Track::setDateAdded(const QDateTime& dateAdded) {
    QMutexLocker lock(&m_qMutex);
    return m_record.setDateAdded(dateAdded);
}

void Track::setDuration(mixxx::Duration duration) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refDuration(), duration)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setDuration(double duration) {
    setDuration(mixxx::Duration::fromSeconds(duration));
}

double Track::getDuration(DurationRounding rounding) const {
    QMutexLocker lock(&m_qMutex);
    switch (rounding) {
    case DurationRounding::SECONDS:
        return std::round(m_record.getMetadata().getDuration().toDoubleSeconds());
    default:
        return m_record.getMetadata().getDuration().toDoubleSeconds();
    }
}

QString Track::getDurationText(mixxx::Duration::Precision precision) const {
    double duration;
    if (precision == mixxx::Duration::Precision::SECONDS) {
        // Round to full seconds before formatting for consistency:
        // getDurationText() should always display the same number
        // as getDuration(DurationRounding::SECONDS) = getDurationInt()
        duration = getDuration(DurationRounding::SECONDS);
    } else {
        duration = getDuration(DurationRounding::NONE);
    }
    return mixxx::Duration::formatTime(duration, precision);
}

QString Track::getTitle() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getTitle();
}

void Track::setTitle(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refTitle(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getArtist() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getArtist();
}

void Track::setArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refArtist(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getAlbum() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getAlbumInfo().getTitle();
}

void Track::setAlbum(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refAlbumInfo().refTitle(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getAlbumArtist()  const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getAlbumInfo().getArtist();
}

void Track::setAlbumArtist(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refAlbumInfo().refArtist(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getYear()  const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getYear();
}

void Track::setYear(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refYear(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getGenre() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getGenre();
}

void Track::setGenre(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refGenre(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getComposer() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getComposer();
}

void Track::setComposer(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refComposer(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getGrouping()  const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getGrouping();
}

void Track::setGrouping(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refGrouping(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getTrackNumber()  const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getTrackNumber();
}

QString Track::getTrackTotal()  const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getTrackTotal();
}

void Track::setTrackNumber(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refTrackNumber(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setTrackTotal(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    QString trimmed(s.trimmed());
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refTrackTotal(), trimmed)) {
        markDirtyAndUnlock(&lock);
    }
}

PlayCounter Track::getPlayCounter() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getPlayCounter();
}

void Track::setPlayCounter(const PlayCounter& playCounter) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refPlayCounter(), playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::updatePlayCounter(bool bPlayed) {
    QMutexLocker lock(&m_qMutex);
    PlayCounter playCounter(m_record.getPlayCounter());
    playCounter.setPlayedAndUpdateTimesPlayed(bPlayed);
    if (compareAndSet(&m_record.refPlayCounter(), playCounter)) {
        markDirtyAndUnlock(&lock);
    }
}

mixxx::RgbColor::optional_t Track::getColor() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getColor();
}

void Track::setColor(mixxx::RgbColor::optional_t color) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refColor(), color)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getComment() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getTrackInfo().getComment();
}

void Track::setComment(const QString& s) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refTrackInfo().refComment(), s)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getType() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getFileType();
}

void Track::setType(const QString& sType) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refFileType(), sType)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::setSampleRate(int iSampleRate) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refSampleRate(), mixxx::AudioSignal::SampleRate(iSampleRate))) {
        markDirtyAndUnlock(&lock);
    }
}

int Track::getSampleRate() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getSampleRate();
}

void Track::setChannels(int iChannels) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refChannels(), mixxx::AudioSignal::ChannelCount(iChannels))) {
        markDirtyAndUnlock(&lock);
    }
}

int Track::getChannels() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getChannels();
}

int Track::getBitrate() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getMetadata().getBitrate();
}

QString Track::getBitrateText() const {
    return QString("%1").arg(getBitrate());
}

void Track::setBitrate(int iBitrate) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refMetadata().refBitrate(), mixxx::AudioSource::Bitrate(iBitrate))) {
        markDirtyAndUnlock(&lock);
    }
}

TrackId Track::getId() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getId();
}

void Track::initId(TrackId id) {
    QMutexLocker lock(&m_qMutex);
    // The track's id must be set only once and immediately after
    // the object has been created.
    VERIFY_OR_DEBUG_ASSERT(!m_record.getId().isValid() || (m_record.getId() == id)) {
        kLogger.warning() << "Cannot change id from"
                << m_record.getId() << "to" << id;
        return; // abort
    }
    m_record.setId(std::move(id));
    // Changing the Id does not make the track dirty because the Id is always
    // generated by the Database itself.
}

void Track::resetId() {
    QMutexLocker lock(&m_qMutex);
    m_record.setId(TrackId());
}

void Track::setURL(const QString& url) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refUrl(), url)) {
        markDirtyAndUnlock(&lock);
    }
}

QString Track::getURL() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getUrl();
}

ConstWaveformPointer Track::getWaveform() const {
    return m_waveform;
}

void Track::setWaveform(ConstWaveformPointer pWaveform) {
    m_waveform = pWaveform;
    emit waveformUpdated();
}

ConstWaveformPointer Track::getWaveformSummary() const {
    return m_waveformSummary;
}

void Track::setWaveformSummary(ConstWaveformPointer pWaveform) {
    m_waveformSummary = pWaveform;
    emit waveformSummaryUpdated();
}

void Track::setCuePoint(CuePosition cue) {
    QMutexLocker lock(&m_qMutex);

    if (!compareAndSet(&m_record.refCuePoint(), cue)) {
        // Nothing changed.
        return;
    }

    // Store the cue point in a load cue
    CuePointer pLoadCue = findCueByType(mixxx::CueType::MainCue);
    double position = cue.getPosition();
    if (position != -1.0) {
        if (!pLoadCue) {
            pLoadCue = CuePointer(new Cue());
            pLoadCue->setTrackId(m_record.getId());
            pLoadCue->setType(mixxx::CueType::MainCue);
            connect(pLoadCue.get(),
                    &Cue::updated,
                    this,
                    &Track::slotCueUpdated);
            m_cuePoints.push_back(pLoadCue);
        }
        pLoadCue->setStartPosition(position);
    } else if (pLoadCue) {
        disconnect(pLoadCue.get(), 0, this, 0);
        m_cuePoints.removeOne(pLoadCue);
    }

    markDirtyAndUnlock(&lock);
    emit cuesUpdated();
}

CuePosition Track::getCuePoint() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getCuePoint();
}

void Track::slotCueUpdated() {
    markDirty();
    emit cuesUpdated();
}

CuePointer Track::createAndAddCue() {
    QMutexLocker lock(&m_qMutex);
    CuePointer pCue(new Cue());
    pCue->setTrackId(m_record.getId());
    connect(pCue.get(), &Cue::updated, this, &Track::slotCueUpdated);
    m_cuePoints.push_back(pCue);
    markDirtyAndUnlock(&lock);
    emit cuesUpdated();
    return pCue;
}

CuePointer Track::findCueByType(mixxx::CueType type) const {
    // This method cannot be used for hotcues because there can be
    // multiple hotcues and this function returns only a single CuePointer.
    VERIFY_OR_DEBUG_ASSERT(type != mixxx::CueType::HotCue) {
        return CuePointer();
    }
    QMutexLocker lock(&m_qMutex);
    for (const CuePointer& pCue: m_cuePoints) {
        if (pCue->getType() == type) {
            return pCue;
        }
    }
    return CuePointer();
}

void Track::removeCue(const CuePointer& pCue) {
    if (pCue == nullptr) {
        return;
    }

    QMutexLocker lock(&m_qMutex);
    disconnect(pCue.get(), 0, this, 0);
    m_cuePoints.removeOne(pCue);
    if (pCue->getType() == mixxx::CueType::MainCue) {
        m_record.setCuePoint(CuePosition());
    }
    markDirtyAndUnlock(&lock);
    emit cuesUpdated();
}

void Track::removeCuesOfType(mixxx::CueType type) {
    QMutexLocker lock(&m_qMutex);
    bool dirty = false;
    QMutableListIterator<CuePointer> it(m_cuePoints);
    while (it.hasNext()) {
        CuePointer pCue = it.next();
        // FIXME: Why does this only work for the Hotcue Type?
        if (pCue->getType() == type) {
            disconnect(pCue.get(), 0, this, 0);
            it.remove();
            dirty = true;
        }
    }
    if (compareAndSet(&m_record.refCuePoint(), CuePosition())) {
        dirty = true;
    }
    if (dirty) {
        markDirtyAndUnlock(&lock);
        emit cuesUpdated();
    }
}

QList<CuePointer> Track::getCuePoints() const {
    QMutexLocker lock(&m_qMutex);
    return m_cuePoints;
}

void Track::setCuePoints(const QList<CuePointer>& cuePoints) {
    //qDebug() << "setCuePoints" << cuePoints.length();
    QMutexLocker lock(&m_qMutex);
    // disconnect existing cue points
    for (const auto& pCue: m_cuePoints) {
        disconnect(pCue.get(), 0, this, 0);
    }
    m_cuePoints = cuePoints;
    // connect new cue points
    for (const auto& pCue: m_cuePoints) {
        connect(pCue.get(), &Cue::updated, this, &Track::slotCueUpdated);
        // Enure that the track IDs are correct
        pCue->setTrackId(m_record.getId());
        // update main cue point
        if (pCue->getType() == mixxx::CueType::MainCue) {
            m_record.setCuePoint(CuePosition(pCue->getPosition()));
        }
    }
    markDirtyAndUnlock(&lock);
    emit cuesUpdated();
}

void Track::importCuePoints(const QList<mixxx::CueInfo>& cueInfos) {
    TrackId trackId;
    mixxx::AudioSignal::SampleRate sampleRate;
    {
        QMutexLocker lock(&m_qMutex);
        trackId = m_record.getId();
        sampleRate = m_record.getMetadata().getSampleRate();
    } // implicitly unlocked when leaving scope

    QList<CuePointer> cuePoints;
    for (const mixxx::CueInfo& cueInfo : cueInfos) {
        CuePointer pCue(new Cue(cueInfo, sampleRate));
        pCue->setTrackId(trackId);
        cuePoints.append(pCue);
    }
    setCuePoints(cuePoints);
}

void Track::markDirty() {
    QMutexLocker lock(&m_qMutex);
    setDirtyAndUnlock(&lock, true);
}

void Track::markClean() {
    QMutexLocker lock(&m_qMutex);
    setDirtyAndUnlock(&lock, false);
}

void Track::markDirtyAndUnlock(QMutexLocker* pLock, bool bDirty) {
    bool result = m_bDirty || bDirty;
    setDirtyAndUnlock(pLock, result);
}

void Track::setDirtyAndUnlock(QMutexLocker* pLock, bool bDirty) {
    const bool dirtyChanged = m_bDirty != bDirty;
    m_bDirty = bDirty;

    const auto trackId = m_record.getId();

    // Unlock before emitting any signals!
    pLock->unlock();

    if (trackId.isValid()) {
        if (dirtyChanged) {
            if (bDirty) {
                emit dirty(trackId);
            } else {
                emit clean(trackId);
            }
        }
        if (bDirty) {
            // Emit a changed signal regardless if this attempted to set us dirty.
            emit changed(trackId);
        }
    }
}

bool Track::isDirty() {
    QMutexLocker lock(&m_qMutex);
    return m_bDirty;
}


void Track::markForMetadataExport() {
    QMutexLocker lock(&m_qMutex);
    m_bMarkedForMetadataExport = true;
    // No need to mark the track as dirty, because this flag
    // is transient and not stored in the database.
}

bool Track::isMarkedForMetadataExport() const {
    QMutexLocker lock(&m_qMutex);
    return m_bMarkedForMetadataExport;
}

int Track::getRating() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getRating();
}

void Track::setRating (int rating) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refRating(), rating)) {
        markDirtyAndUnlock(&lock);
    }
}

void Track::afterKeysUpdated(QMutexLocker* pLock) {
    // New key might be INVALID. We don't care.
    mixxx::track::io::key::ChromaticKey newKey = m_record.getGlobalKey();
    markDirtyAndUnlock(pLock);
    emit keyUpdated(KeyUtils::keyToNumericValue(newKey));
    emit keysUpdated();
}

void Track::setKeys(const Keys& keys) {
    QMutexLocker lock(&m_qMutex);
    m_record.setKeys(keys);
    afterKeysUpdated(&lock);
}

void Track::resetKeys() {
    QMutexLocker lock(&m_qMutex);
    m_record.resetKeys();
    afterKeysUpdated(&lock);
}

Keys Track::getKeys() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getKeys();
}

void Track::setKey(mixxx::track::io::key::ChromaticKey key,
                   mixxx::track::io::key::Source keySource) {
    QMutexLocker lock(&m_qMutex);
    if (m_record.updateGlobalKey(key, keySource)) {
        afterKeysUpdated(&lock);
    }
}

mixxx::track::io::key::ChromaticKey Track::getKey() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getGlobalKey();
}

QString Track::getKeyText() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getGlobalKeyText();
}

void Track::setKeyText(const QString& keyText,
                       mixxx::track::io::key::Source keySource) {
    QMutexLocker lock(&m_qMutex);
    if (m_record.updateGlobalKeyText(keyText, keySource)) {
        afterKeysUpdated(&lock);
    }
}

void Track::setBpmLocked(bool bpmLocked) {
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refBpmLocked(), bpmLocked)) {
        markDirtyAndUnlock(&lock);
    }
}

bool Track::isBpmLocked() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getBpmLocked();
}

void Track::setCoverInfo(const CoverInfoRelative& coverInfo) {
    DEBUG_ASSERT((coverInfo.type != CoverInfo::METADATA) || coverInfo.coverLocation.isEmpty());
    DEBUG_ASSERT((coverInfo.source != CoverInfo::UNKNOWN) || (coverInfo.type == CoverInfo::NONE));
    QMutexLocker lock(&m_qMutex);
    if (compareAndSet(&m_record.refCoverInfo(), coverInfo)) {
        markDirtyAndUnlock(&lock);
        emit coverArtUpdated();
    }
}

CoverInfoRelative Track::getCoverInfo() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getCoverInfo();
}

CoverInfo Track::getCoverInfoWithLocation() const {
    QMutexLocker lock(&m_qMutex);
    return CoverInfo(m_record.getCoverInfo(), m_fileInfo.location());
}

quint16 Track::getCoverHash() const {
    QMutexLocker lock(&m_qMutex);
    return m_record.getCoverInfo().hash;
}

ExportTrackMetadataResult Track::exportMetadata(
        mixxx::MetadataSourcePointer pMetadataSource) {
    VERIFY_OR_DEBUG_ASSERT(pMetadataSource) {
        kLogger.warning()
                << "Cannot export track metadata:"
                << getLocation();
        return ExportTrackMetadataResult::Failed;
    }
    // Locking shouldn't be necessary here, because this function will
    // be called after all references to the object have been dropped.
    // But it doesn't hurt much, so let's play it safe ;)
    QMutexLocker lock(&m_qMutex);
    // TODO(XXX): m_record.getMetadataSynchronized() currently is a
    // boolean flag, but it should become a time stamp in the future.
    // We could take this time stamp and the file's last modification
    // time stamp into account and might decide to skip importing
    // the metadata again.
    if (!m_bMarkedForMetadataExport && !m_record.getMetadataSynchronized()) {
        // If the metadata has never been imported from file tags it
        // must be exported explicitly once. This ensures that we don't
        // overwrite existing file tags with completely different
        // information.
        kLogger.info()
                << "Skip exporting of unsynchronized track metadata:"
                << getLocation();
        // abort
        return ExportTrackMetadataResult::Skipped;
    }
    // Normalize metadata before exporting to adjust the precision of
    // floating values, ... Otherwise the following comparisons may
    // repeatedly indicate that values have changed only due to
    // rounding errors.
    m_record.refMetadata().normalizeBeforeExport();
    // Check if the metadata has actually been modified. Otherwise
    // we don't need to write it back. Exporting unmodified metadata
    // would needlessly update the file's time stamp and should be
    // avoided. Since we don't know in which state the file's metadata
    // is we import it again into a temporary variable.
    mixxx::TrackMetadata importedFromFile;
    if ((pMetadataSource->importTrackMetadataAndCoverImage(&importedFromFile, nullptr).first ==
            mixxx::MetadataSource::ImportResult::Succeeded)) {
        // Prevent overwriting any file tags that are not yet stored in the
        // library database!
        m_record.mergeImportedMetadata(importedFromFile);
        // Finally the track's current metadata and the imported/adjusted metadata
        // can be compared for differences to decide whether the tags in the file
        // would change if we perform the write operation. This function will also
        // copy all extra properties that are not (yet) stored in the library before
        // checking for differences! If an export has been requested explicitly then
        // we will continue even if no differences are detected.
        // NOTE(uklotzde, 2020-01-05): Detection of modified bpm values is restricted
        // to integer precision to avoid re-exporting of unmodified ID3 tags in case
        // of fractional bpm values. As a consequence small changes in bpm values
        // cannot be detected and file tags with fractional values might not be
        // updated as expected! In these edge cases users need to explicitly
        // trigger the re-export of file tags or they could modify other metadata
        // properties.
        if (!m_bMarkedForMetadataExport &&
                !m_record.getMetadata().anyFileTagsModified(
                        importedFromFile,
                        mixxx::Bpm::Comparison::Integer))  {
            // The file tags are in-sync with the track's metadata and don't need
            // to be updated.
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                            << "Skip exporting of unmodified track metadata into file:"
                            << getLocation();
            }
            // abort
            return ExportTrackMetadataResult::Skipped;
        }
    } else {
        // The file doesn't contain any tags yet or it might be missing, unreadable,
        // or corrupt.
        if (m_bMarkedForMetadataExport) {
            kLogger.info()
                    << "Adding or overwriting tags after failure to import tags from file:"
                    << getLocation();
            // ...and continue
        } else {
            kLogger.warning()
                    << "Skip exporting of track metadata after failure to import tags from file:"
                    << getLocation();
            // abort
            return ExportTrackMetadataResult::Skipped;
        }
    }
    // The track's metadata will be exported instantly. The export should
    // only be tried once so we reset the marker flag.
    m_bMarkedForMetadataExport = false;
    kLogger.debug()
            << "Old metadata (imported)"
            << importedFromFile;
    kLogger.debug()
            << "New metadata (modified)"
            << m_record.getMetadata();
    const auto trackMetadataExported =
            pMetadataSource->exportTrackMetadata(m_record.getMetadata());
    if (trackMetadataExported.first == mixxx::MetadataSource::ExportResult::Succeeded) {
        // After successfully exporting the metadata we record the fact
        // that now the file tags and the track's metadata are in sync.
        // This information (flag or time stamp) is stored in the database.
        // The database update will follow immediately after returning from
        // this operation!
        // TODO(XXX): Replace bool with QDateTime
        DEBUG_ASSERT(!trackMetadataExported.second.isNull());
        //pTrack->setMetadataSynchronized(trackMetadataExported.second);
        m_record.setMetadataSynchronized(!trackMetadataExported.second.isNull());
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Exported track metadata:"
                    << getLocation();
        }
        return ExportTrackMetadataResult::Succeeded;
    } else {
        kLogger.warning()
                << "Failed to export track metadata:"
                << getLocation();
        return ExportTrackMetadataResult::Failed;
    }
}
