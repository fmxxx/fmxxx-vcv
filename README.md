# Modules

## Utilities, signal processors
+ Tuner (Semis + Cents + Ratio) [Poly]
	Knob snapping to integers can be toggled on/off.
+ Fract (Numerator / Denominator) [Poly]
	Knob snapping to integers can be toggled on/off. When on, the module is useful for generating non integer ratios that sound musical (Some FM synths tune to 1/10ths or 1/4ths, for example)
+ Mathy (+, -, *, /, pow, mod) [Poly]
+ Phase Distortion [Poly]
	Can be placed before a Phase_WT to modulate the waveform shape or before a Buffr to change the write/read heads for intereseting effects.
	Can be used as a waveshaper to process audio singals, in that case use the Adapter module (AUDIO -> Bi to Uni -> PD -> Uni to Bi). If you notice clicking in some algorithms just lower the volume before it goes into the module.
+ Adapter
	Adapts voltages from Unipolar to Bipolar and vice-versa.
+ Mix VCA
	An 8 channel polyphonic mixer that can work as 8 individual VCAs + one on the master. Meant mostly for CV so it has no saturation stage, only a hard clipper (-12V, 12V).


## Oscillators 
+ Phasor [Poly]
	Tuned phase generator with phase offset and modulation.
+ Phase_WT [Poly]
 	Takes a phase input instead of V/Oct. Has a built in phase distortion stage
+ RipplerX port (All models loaded into memory) [Poly]

## Recorders

+ Buffr [Mono]

## Controllers

+ XY pad (multiple dots) [Mono]
+ XY pad ++ (each dot has its own knob) [Mono]

