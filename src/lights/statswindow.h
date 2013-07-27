#ifndef STATSWINDOW_H
#define STATSWINDOW_H

class StatsWindow {
  public:
    StatsWindow(int windowSize)
            : m_windowSize(windowSize),
              m_window(windowSize) {
    }

    virtual ~StatsWindow() {
    }

    void appendValue(qreal value) {
        if (m_window.size() == m_windowSize) {
            m_window_sum -= m_window.last();
            m_window.popLast();
        }
        m_window.append(value);
        m_window_sum += value;
    }

    void clear() {
        m_window.clear();
        m_window_sum = 0;
    }

    qreal getAverage() {
        if (m_window.size() > 0)
            return m_window_sum / m_window.size();
        return 0;
    }

    qreal getVariance() {
        if (m_window.size() == 0)
            return 0;

        qreal average = getAverage();
        qreal variance_sum = 0;
        foreach (qreal value, m_window) {
            qreal diff = value - average;
            variance_sum += diff*diff;
        }
        return variance_sum / m_window.size();
    }

    qreal getStdDev() {
        qreal variance = getVariance();
        return sqrt(variance);
    }

  private:
    const int m_windowSize;
    QVector<qreal> m_window;
    qreal m_window_sum;

}


#endif /* STATSWINDOW_H */
