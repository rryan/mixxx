#ifndef DMXETH_H
#define DMXETH_H

#include <arpa/inet.h>

#define KINET_UDP_PORT 6038
#define KINET_VERSION 0x0001
#define KINET_MAGIC 0x4adc0104
#define KTYPE_DMXOUT 0x0101
#define KTYPE_DMXOUT2 0x0108
#ifndef Byte
#define Byte unsigned char
#endif
#ifndef Word
#define Word unsigned short
#endif
#ifndef DWord
#define DWord unsigned int
#endif


// typedef struct {
//     unsigned int magic;
//     unsigned short ver;
//     unsigned short type;
//     unsigned int seq;
// } KiNET_Hdr;

// typedef struct {
//     KiNET_Hdr hdr;
//     unsigned char port;
//     unsigned char flags;
//     unsigned short timerVal;
//     unsigned int uni;
//     //DWord numChannels;
//     // need to add dmx data here
// } KiNET_DMXout;

typedef struct {
  DWord magic;
  Word ver;
  Word type;
  DWord seq;
} KiNET_Hdr;

typedef struct {
  KiNET_Hdr hdr;
  Byte port;
  Byte padding;
  Word flags;
  DWord timerVal;
  Byte uni;
  //DWord numChannels;
  // need to add dmx data here
} KiNET_DMXout;

typedef struct {
    DWord magic; // 0x4adc0104
    Word ver; // 0x0001
    Word type; // 0x0108
    DWord seq; // 0x00000000
} KiNET_Hdr2;

typedef struct {
    KiNET_Hdr2 hdr;
    Byte port;     // 0x00
    Byte padding;  // 0x00
    Word flags;    // 0x0000
    Byte uni;      //
    Byte padding2; // 0x00
    Byte padding3; // 0x00
    Byte padding4; // 0x00
    Byte padding5; // 0x96
    Byte padding6; // 0x00
    Byte padding7; // 0x00
    Byte padding8; // 0x00
} KiNET_DMXout2;

typedef struct {
    int sock;
    struct sockaddr_in destsa;
    unsigned char dmx_state[1024];
    int type;
} DMX_Handle;

DMX_Handle* new_dmx_handle(char* address, int type);
void free_dmx_handle(DMX_Handle* pHandle);
void send_dmx_data(DMX_Handle* pHandle, unsigned char *data, int dlen, Byte uni=-1);
void set_dmx_color(DMX_Handle* pHandle, int light,
                   unsigned char r, unsigned char g, unsigned char b);
void publish_dmx_update(DMX_Handle* pHandle, Byte uni);

#endif /* DMXETH_H */

