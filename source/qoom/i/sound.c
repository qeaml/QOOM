#include "qoom/i/system.h"
#include "qoom/i/sound.h"
#include <SDL2/SDL_mixer.h>

// TODO: does not currently produce any sounds.

// Needed for calling the actual sound output.
#define SAMPLECOUNT 512
#define NUM_CHANNELS 8
#define SAMPLERATE 11025     // Hz
#define SAMPLESIZE AUDIO_S16 // 16bit

void I_InitSound() {
  if(Mix_Init(MIX_INIT_MID) == 0)
    I_Error("can't initialize mixer: %s", Mix_GetError());

  if(Mix_OpenAudio(SAMPLERATE, SAMPLESIZE, 2, SAMPLECOUNT) != 0)
    I_Error("can't initialize audio device: %s", Mix_GetError());
}

void I_UpdateSound() {}
void I_SubmitSound() {}

void I_ShutdownSound() {
  Mix_Quit();
}

void I_SetChannels() {}

int I_GetSfxLumpNum(sfxinfo_t *sfxinfo) {
  return 0;
}

int I_StartSound(int id, int vol, int sep, int pitch, int priority) {
  return 0;
}

void I_StopSound(int handle) {}

int I_SoundIsPlaying(int handle) {
  return 0;
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch) {}

void I_InitMusic(void) {}
void I_ShutdownMusic(void) {}

void I_SetMusicVolume(int volume) {}

void I_PauseSong(int handle) {}
void I_ResumeSong(int handle) {}

int I_RegisterSong(void *data) {
  return 0;
}

void I_PlaySong(int handle, int looping) {}

void I_StopSong(int handle) {}

void I_UnRegisterSong(int handle) {}
