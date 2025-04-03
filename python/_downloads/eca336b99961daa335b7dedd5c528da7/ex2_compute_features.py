"""
Compute features
================
"""

import os

import matplotlib.pyplot as plt
import numpy as np
import librosa
import librosa.display
import rtvamp

# set VAMP_PATH to rtvamp package dir to find example plugins
os.environ["VAMP_PATH"] = os.path.dirname(rtvamp.__file__)

#%%
# Load sample audio data
# ----------------------
y, sr = librosa.load(librosa.ex("trumpet"))

#%%
# Compute features with rtvamp
# ----------------------------
t_rms, y_rms = rtvamp.compute_features(y, sr, plugin="example-plugin:rms", blocksize=256)
t_sro, y_sro = rtvamp.compute_features(y, sr, plugin="example-plugin:spectralrolloff", parameter={"rolloff": 0.5})

#%%
# Plot features with spectrogram
# ------------------------------
fig, ax = plt.subplots(nrows=3, ncols=1, sharex=True, tight_layout=True)
D = librosa.amplitude_to_db(np.abs(librosa.stft(y)), ref=np.max)
librosa.display.specshow(D, x_axis="time", y_axis="linear", sr=sr, ax=ax[0])
ax[0].set(title="STFT")

ax[1].plot(t_rms, y_rms[0])
ax[1].set(title="RMS")

ax[2].plot(t_sro, y_sro[0])
ax[2].set(title="Spectral Rolloff")

plt.show()
