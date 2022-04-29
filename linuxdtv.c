// Compile: gcc -o linuxdtv.c linuxdtv
// Usage: ./linuxdtv /dev/dvb/adapter0/frontend0

#include <fcntl.h>
#include <linux/dvb/frontend.h>
#include <stdio.h>
#include <sys/ioctl.h>

typedef struct {
  enum fe_caps cap;
  const char* str;
} enum_caps;

typedef struct {
  enum fe_delivery_system delsys;
  const char* str;
} enum_delsys;

#define enum_caps_ctor(x) { .cap = x, .str = #x }
#define enum_delsys_ctor(x) { .delsys = x, .str = #x }
#define verify_flag(x, y) (y != 0 && x & y) || (y == 0 && x == y)

// https://elixir.bootlin.com/linux/v5.10.112/source/include/uapi/linux/dvb/frontend.h#L33
static enum_caps caps[] = {
  enum_caps_ctor(FE_IS_STUPID),
  enum_caps_ctor(FE_CAN_INVERSION_AUTO),
  enum_caps_ctor(FE_CAN_FEC_1_2),
  enum_caps_ctor(FE_CAN_FEC_2_3),
  enum_caps_ctor(FE_CAN_FEC_3_4),
  enum_caps_ctor(FE_CAN_FEC_4_5),
  enum_caps_ctor(FE_CAN_FEC_5_6),
  enum_caps_ctor(FE_CAN_FEC_6_7),
  enum_caps_ctor(FE_CAN_FEC_7_8),
  enum_caps_ctor(FE_CAN_FEC_8_9),
  enum_caps_ctor(FE_CAN_FEC_AUTO),
  enum_caps_ctor(FE_CAN_QPSK),
  enum_caps_ctor(FE_CAN_QAM_16),
  enum_caps_ctor(FE_CAN_QAM_32),
  enum_caps_ctor(FE_CAN_QAM_64),
  enum_caps_ctor(FE_CAN_QAM_128),
  enum_caps_ctor(FE_CAN_QAM_256),
  enum_caps_ctor(FE_CAN_QAM_AUTO),
  enum_caps_ctor(FE_CAN_TRANSMISSION_MODE_AUTO),
  enum_caps_ctor(FE_CAN_BANDWIDTH_AUTO),
  enum_caps_ctor(FE_CAN_GUARD_INTERVAL_AUTO),
  enum_caps_ctor(FE_CAN_HIERARCHY_AUTO),
  enum_caps_ctor(FE_CAN_8VSB),
  enum_caps_ctor(FE_CAN_16VSB),
  enum_caps_ctor(FE_HAS_EXTENDED_CAPS),
  enum_caps_ctor(FE_CAN_MULTISTREAM),
  enum_caps_ctor(FE_CAN_TURBO_FEC),
  enum_caps_ctor(FE_CAN_2G_MODULATION),
  enum_caps_ctor(FE_NEEDS_BENDING),
  enum_caps_ctor(FE_CAN_RECOVER),
  enum_caps_ctor(FE_CAN_MUTE_TS),
};

// https://elixir.bootlin.com/linux/v5.10.112/source/include/uapi/linux/dvb/frontend.h#L628
static enum_delsys delsyss[] = {
  enum_delsys_ctor(SYS_UNDEFINED),
  enum_delsys_ctor(SYS_DVBC_ANNEX_A),
  enum_delsys_ctor(SYS_DVBC_ANNEX_B),
  enum_delsys_ctor(SYS_DVBT),
  enum_delsys_ctor(SYS_DSS),
  enum_delsys_ctor(SYS_DVBS),
  enum_delsys_ctor(SYS_DVBS2),
  enum_delsys_ctor(SYS_DVBH),
  enum_delsys_ctor(SYS_ISDBT),
  enum_delsys_ctor(SYS_ISDBS),
  enum_delsys_ctor(SYS_ISDBC),
  enum_delsys_ctor(SYS_ATSC),
  enum_delsys_ctor(SYS_ATSCMH),
  enum_delsys_ctor(SYS_DTMB),
  enum_delsys_ctor(SYS_CMMB),
  enum_delsys_ctor(SYS_DAB),
  enum_delsys_ctor(SYS_DVBT2),
  enum_delsys_ctor(SYS_TURBO),
  enum_delsys_ctor(SYS_DVBC_ANNEX_C),
};

int main(int argc, char** argv) {

  //////////////////////// OPENING FRONTEND ////////////////////////

  if (argc != 2) {
    fprintf(stderr, "No frontend specified!\n");
    fprintf(stderr, "Use the following format: %s /dev/dvb/adapterX/frontendY\n", argv[0]);
    return -1;
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("Unable to open frontend");
    return -1;
  }

  //////////////////////// FE_GET_INFO ////////////////////////

  // https://elixir.bootlin.com/linux/v5.10.112/source/include/uapi/linux/dvb/frontend.h#L146
  struct dvb_frontend_info frontend_info;

  if (ioctl(fd, FE_GET_INFO, &frontend_info) < 0) {
    perror("Unable to query ioctl FE_GET_INFO");
    return -1;
  }

  printf("%s\n", frontend_info.name);
  printf("\n");

  printf("Frequency properties:\n");
  printf("%s: %d\n", "FREQUENCY MIN", frontend_info.frequency_min);
  printf("%s: %d\n", "FREQUENCY MAX", frontend_info.frequency_max);
  printf("%s: %d\n", "FREQUENCY STEPSIZE", frontend_info.frequency_stepsize);
  printf("%s: %d\n", "FREQUENCY TOLERANCE", frontend_info.frequency_tolerance);
  printf("\n");

  printf("Symbol rate properties:\n");
  printf("%s: %d\n", "SYMBOL RATE MIN", frontend_info.symbol_rate_min);
  printf("%s: %d\n", "SYMBOL RATE MAX", frontend_info.symbol_rate_max);
  printf("%s: %d\n", "SYMBOL RATE TOLERANCE", frontend_info.symbol_rate_tolerance);
  printf("\n");

  printf("Supported capabilities:\n");
  for (int i = 0, s = sizeof(caps)/sizeof(enum_caps); i < s; ++i) {
    printf("%s: %d\n", caps[i].str, verify_flag(frontend_info.caps, caps[i].cap));
  }
  printf("\n");

  //////////////////////// FE_GET_PROPERTY ////////////////////////

  // https://elixir.bootlin.com/linux/v5.10.112/source/drivers/media/dvb-core/dvb_frontend.c#L1346
  struct dtv_properties properties = {
    .num = 2, .props = (struct dtv_property[]) {
      { .cmd = DTV_ENUM_DELSYS },
      { .cmd = DTV_API_VERSION },
    }
  };

  if (ioctl(fd, FE_GET_PROPERTY, &properties) < 0) {
    perror("Unable to query ioctl FE_GET_PROPERTY");
    return -1;
  }

  printf("Supported delivery systems:\n");
  for (int i = 0, s = properties.props[0].u.buffer.len; i < s; ++i) {
    printf("%s\n", delsyss[properties.props[0].u.buffer.data[i]].str);
  }
  printf("\n");

  int dvb_api_mayor = properties.props[1].u.data >> 8 & 0xff;
  int dvb_api_minor = properties.props[1].u.data & 0xff;
  printf("Linux DVB API Version: %d.%d\n", dvb_api_mayor, dvb_api_minor);

  return 0;
}
