#ifndef WTSYNTH_H
#define WTSYNTH_H

#ifndef LFOCB
#define LFOCB 960
#endif

/* return codes */
enum
{
	WTS_OK,
	WTS_FAIL,
	WTS_FILE_NOT_SUPPORTED,
	WTS_FILE_NOT_EXIST,
	WTS_NO_MORE,
	WTS_NOT_IMPLEMENTED
};

#ifdef __cplusplus
extern "C" {
#endif

/* construct and destruct functions */
int wtsynth_Init(void);
int wtsynth_Kill(void);

/* patch and preset handler routines */
int wtsynth_GetPatchInfo(char *fn, void (*callback)(char *));
int wtsynth_LoadPatch(char *fn, long *handle);
int wtsynth_UnloadPatch(long handle);
int wtsynth_RemoveAllPatches(void);
int wtsynth_GetPatchFilename(int handle, char *fn);
int wtsynth_GetPreset(int ndx, char *patchname, int *bank, int *preset);

/* midi handler routines */
int wtsynth_NoteOn(int chan, int note, int velocity);
int wtsynth_NoteOff(int chan, int note);
int wtsynth_CtrlChange(int chan, int ctl, int value);
int wtsynth_SetPatch(int chan, int preset);
int wtsynth_SetBank(int chan, int bank);
int wtsynth_Reset(void);
int wtsynth_SetPitchWheel(int chan, int amount);
void wtsynth_AllNotesOff(void);

/* midi parser routines */
int wtsynth_HandleMidiData(unsigned char *data, int len);
int wtsynth_HandleMidiBlock(unsigned char *data, int len);

/* audio output handler */
int wtsynth_GetAudioPacket(short *data);

/* settings */
int wtsynth_Config(int setting, uint32_t value);

/* sysex handling */
void wtsynth_Sysex(uint8_t data);

/* power handling */
void wtsynth_ActiveState(uint8_t active);

/* bar graph display chan 0-15, level 0-7 */
void wtsynth_SetBar(uint8_t chan, uint8_t level);

/* optimisations */
#ifdef PREFETCH
void wtsynth_PrefetchSamples(void);
#endif

#ifdef __cplusplus
};
#endif

#endif
