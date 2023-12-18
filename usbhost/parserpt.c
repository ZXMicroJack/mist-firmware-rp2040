typedef struct {
  uint8_t usages[256];
  uint8_t nr_usages;
  uint8_t report_size;
  uint8_t report_count;
  uint8_t usage_page;
  uint8_t report_id;
  
  uint8_t report_len;
  uint16_t report_len_bits;
  
  uint8_t off_x, off_y, off_butts, nr_butts;
  uint8_t candidate_report_id;
} joypad_report_t;

uint8_t roundup8(uint8_t d) {
  return ((d + 7) >> 3) << 3;
}

void prd_init(joypad_report_t *s) {
  memset(s, 0x00, sizeof (joypad_report_t));
  s->off_x = 0xff;
  s->off_y = 0xff;
  s->off_butts = 0xff;
  s->candidate_report_id = 0xff;
  s->report_id = 0xff;
}

void prd_process(joypad_report_t *s) {
  if (s->report_size == 8 && s->nr_usages) {
    for (int j=0; j<s->nr_usages; j++) {
      if (s->usages[j] == HID_USAGE_DESKTOP_X && s->off_x == 0xff) s->off_x = s->report_len + j;
      else if (s->usages[j] == HID_USAGE_DESKTOP_Y && s->off_y == 0xff) s->off_y = s->report_len + j;
#ifdef DEBUG
      else printf("usages[j] = 0x%02X\n", s->usages[j]);
#endif
    }
  } else if (s->nr_usages) {
    for (int j=0; j<s->nr_usages; j++) {
      if (s->usages[j] == HID_USAGE_DESKTOP_HAT_SWITCH && s->off_butts == 0xff) {
        s->off_butts = (s->report_len_bits + j * s->report_size) >> 3;
        s->nr_butts = s->report_size;
      }
#ifdef DEBUG
      else printf("usages[j] = 0x%02X\n", s->usages[j]);
#endif
    }
    
  }
  
  if (s->usage_page == HID_USAGE_PAGE_BUTTON && s->off_butts == 0xff) {
    s->off_butts = s->report_len;
  }
  
#ifdef DEBUG
  printf("Adding %d bytes\n", roundup8(s->report_count * s->report_size) >> 3);
#endif
  s->report_len += roundup8(s->report_count * s->report_size) >> 3;
  s->report_len_bits += s->report_count * s->report_size;
  s->report_count = s->report_size = s->nr_usages = 0;
  if (s->off_x != 0xff && s->report_id == 0xff) {
    s->report_id = s->candidate_report_id;
  }
}

void prd_Parse(uint8_t *desc, uint32_t desclen, uint8_t *off_x, uint8_t *off_y, uint8_t *off_butts) {
  int i=0, collection = 0;
  joypad_report_t jr;
  
  *off_x = 0xff;
  *off_y = 0xff;
  *off_butts = 0xff;
  
  prd_init(&jr);
  
	while (i<desclen) {
    uint8_t tag, type, len;

    tag = desc[i] >> 4;
    type = (desc[i] >> 2) & 0x3;
    len = desc[i] & 0x3;

    if (type == RI_TYPE_MAIN && tag == RI_MAIN_COLLECTION) { collection ++; jr.nr_usages = jr.report_count = jr.report_size = 0; }

    // start of a new report description
    if (type == RI_TYPE_MAIN && tag == RI_MAIN_COLLECTION && len == 1 && desc[i+1] == HID_COLLECTION_LOGICAL) {
#ifdef DEBUG
      printf("off_x %d off_y %d len %d lenbits %d\n", jr.off_x, jr.off_y, jr.report_len, jr.report_len_bits);
#endif
      jr.nr_usages = jr.report_count = jr.report_size = 0;
      jr.report_len = 0;
      jr.report_len_bits = 0;
    }
    
    // end collection
    if (type == RI_TYPE_MAIN && tag == RI_MAIN_COLLECTION_END) {
      prd_process(&jr);
      collection --;
    }

    // collect usages
    if (type == RI_TYPE_GLOBAL && tag == RI_GLOBAL_USAGE_PAGE && len == 1) jr.usage_page = desc[i+1];
    if (type == RI_TYPE_LOCAL && tag == RI_LOCAL_USAGE) jr.usages[jr.nr_usages++] = desc[i+1];

    // get report size
    if (type == RI_TYPE_GLOBAL && tag == RI_GLOBAL_REPORT_SIZE) {
      if (jr.report_size) {
        prd_process(&jr);
        jr.report_size = 0;
        jr.nr_usages = 0;
      }
      jr.report_size = desc[i+1];
    }
    if (type == RI_TYPE_GLOBAL && tag == RI_GLOBAL_REPORT_COUNT) {
      if (jr.report_count) {
        prd_process(&jr);
        jr.report_size = 0;
        jr.nr_usages = 0;
      }
      jr.report_count = desc[i+1];
    }
 
    // report id
    if (type == RI_TYPE_GLOBAL && tag == RI_GLOBAL_REPORT_ID) jr.candidate_report_id = desc[i+1];
    
    i += 1 + len;
	}
	
#ifdef DEBUG
	printf("off_x %d off_y %d butts %d nrbutts %d len %d bits %d report 0x%02x\n",
        jr.off_x, jr.off_y, jr.off_butts, jr.nr_butts,
        jr.report_len, jr.report_len_bits, jr.report_id);
#endif
  *off_x = jr.off_x;
  *off_y = jr.off_y;
  *off_butts = jr.off_butts;
}
