/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "../../../webrtc/common_audio/vad/include/webrtc_vad.h"

#include <stdlib.h>
#include <string.h>

#include "../../../webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#include "../../../webrtc/common_audio/vad/vad_core.h"
#include "../../../webrtc/typedefs.h"

static const int kInitCheck = 42;
static const int kValidRates[] = { 8000, 16000, 32000, 48000 };
static const size_t kRatesSize = sizeof(kValidRates) / sizeof(*kValidRates);
static const int kMaxFrameLengthMs = 30;

VadInst* WebRtcVad_Create() {
  VadInstT* self = (VadInstT*)malloc(sizeof(VadInstT));

  WebRtcSpl_Init();
  self->init_flag = 0;

  return (VadInst*)self;
}

void WebRtcVad_Free(VadInst* handle) {
  free(handle);
}

// TODO(bjornv): Move WebRtcVad_InitCore() code here.
int WebRtcVad_Init(VadInst* handle) {
  // Initialize the core VAD component.
  return WebRtcVad_InitCore((VadInstT*) handle);
}

int WebRtcVad_set_mode(VadInst* handle, int mode) {
  VadInstT* self = (VadInstT*) handle;

  if (handle == NULL) {
    return -1;
  }
  if (self->init_flag != kInitCheck) {
    return -1;
  }

  return WebRtcVad_set_mode_core(self, mode);
}

VadResult WebRtcVad_Process(VadInst* handle, int fs, const int16_t* audio_frame,
                      size_t frame_length) {
  VadResult result;
  result.total_energy = 0;

  int vad = -1;
  VadInstT* self = (VadInstT*) handle;

  if (handle == NULL) {
    vad = -1;
    result.vad = vad;
    return result;

  }

  if (self->init_flag != kInitCheck) {
    vad = -1;
    result.vad = vad;
    return result;
  }
  if (audio_frame == NULL) {
    vad = -1;
    result.vad = vad;
    return result;
  }
  if (WebRtcVad_ValidRateAndFrameLength(fs, frame_length) != 0) {
    vad = -1;
    result.vad = vad;
    return result;
  }


  VadResultT vad_result;
  if (fs == 48000) {
      vad_result = WebRtcVad_CalcVad48khz(self, audio_frame, frame_length);
  } else if (fs == 32000) {
    vad_result = WebRtcVad_CalcVad32khz(self, audio_frame, frame_length);
  } else if (fs == 16000) {
    vad_result = WebRtcVad_CalcVad16khz(self, audio_frame, frame_length);
  } else if (fs == 8000) {
    vad_result = WebRtcVad_CalcVad8khz(self, audio_frame, frame_length);
  }

  if (vad_result.vad > 0) {
    vad_result.vad = 1;
  }

  result.vad = vad_result.vad;
  result.total_energy = vad_result.total_energy;
  
  return result;
}

int WebRtcVad_ValidRateAndFrameLength(int rate, size_t frame_length) {
  int return_value = -1;
  size_t i;
  int valid_length_ms;
  size_t valid_length;

  // We only allow 10, 20 or 30 ms frames. Loop through valid frame rates and
  // see if we have a matching pair.
  for (i = 0; i < kRatesSize; i++) {
    if (kValidRates[i] == rate) {
      for (valid_length_ms = 10; valid_length_ms <= kMaxFrameLengthMs;
          valid_length_ms += 10) {
        valid_length = (size_t)(kValidRates[i] / 1000 * valid_length_ms);
        if (frame_length == valid_length) {
          return_value = 0;
          break;
        }
      }
      break;
    }
  }

  return return_value;
}

void WebRtcVad_setThreshold(VadInst* handle, int threshold)
{
	VadInstT* self = (VadInstT*) handle;
	WebRtcVad_SetThresholdCore(self, threshold);
}
