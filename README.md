# Modules

## Utilities, signal processors
+ Tuner (Semis + Cents + Ratio) [Poly]
	Knob snapping to integers can be toggled on/off.
+ Fract (Numerator / Denominator) [Poly]
	Knob snapping to integers can be toggled on/off. When on, the module is useful for generating non integer ratios that sound musical (Some FM synths tune to 1/10ths or 1/4ths, for example)
+ Mathy (+, -, *, /, pow, mod) [Poly]
+ Phase Distortion [Poly]
	Can be placed before a Phase_WT to modulate the waveform shape or before a Buffr to change the write/read heads for intereseting effects.
	Can be used to process audio singals, in that case, make sure to offset them by +5V, scale it down a little, then offset back down -5V.
	


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

