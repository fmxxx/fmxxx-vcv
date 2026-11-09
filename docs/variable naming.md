- Ordinary variables:
  - Use lower camel case and describe the value, including its unit when useful.
  - Example: `waveSizeSamples` holds a sample count.

- Parameter value that is modulated:
  - Use the "Param" suffix when the value is read directly from a parameter and then combined with modulation.
  - Example: `ratioParam` comes from `RATIO_PARAM` and is modulated with `ratioMod`.

- Transformed or scaled parameter value used with modulation:
  - Use "Base" for the parameter-derived baseline, and include its resulting unit or meaning when useful.
  - Example: `freqBaseHz` is converted from `FREQ_PARAM` to Hz, then scaled by the V/Oct input.

- Transformed parameter value without modulation:
  - Name the resulting value for what it represents; it doesn't need a "Param" or "Base" suffix.
  - Example: `waveSizeSamples` is derived from `WAVESIZE_PARAM` by converting it from 2^n to a sample count.

- Direct parameter value without modulation:
  - Name the value for its meaning without a "Param" suffix.
  - Example: `phaseOffset` comes directly from `PHASE_OFFSET_PARAM` and is applied without modulation.

- Modulations:
  - Use the "Mod" suffix for a processed CV value, e.g. scaling from (-5V, +5V) to (-1, 1).
  - Example: `ratioMod` is converted to the ratio scale and added to `ratioParam`.

- Modulation amount parameter:
  - Use the "CVAmtParam" suffix for an attenuverter that scales incoming CV.
  - Example: `pdCVAmtParam` comes from `PD_CV_AMT_PARAM` and scales `inputs[PD_INPUT]`.
	
- Boolean variables:
  - Name it as a clear state or choice, commonly using is, has, or do.
  - Example: `doMorph` stores the on/off choice from `DO_MORPH_PARAM`.
