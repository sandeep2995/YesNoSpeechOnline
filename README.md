# YesNoSpeechOnline
recognizing "yes" or "no" using recording module

-----------------------------
Execution Instructions:
-----------------------------

Go to directory YesNoMyCode.

set the values as desired in "config.h" file such as file name to store the speech, path of Recording Module, duration in seconds to allow recording the speech.

Inputs--->contains all inputs used for the testing (e.g. n1, y1, n2, y2, n3, y3, ...)

Outputs--->contains all outputs generated for the inputs. (e.g. outn1, outy1, ScaledInpSpeechn1, outn2, outy2, ScaledInpSpeechy2, outn3, outy3, ScaledInpSpeechn3, ...)

Note the format of saving. example: n1.txt and n1.wav are stored in Inputs while their corresponding outputs outn1.txt, ScaledInpSpeechn1.txt are stored in Outputs.

Now go back to YesNoMyCode directory (i.e. parent directory of this project)

open the program solution(YesNoMyCode.sln) in visual studio, execute it by pressing F5, 
then it displays "Start Recording.........", you can start recording your signal. when the duration(as mentioned in "config.h" file) is over(say 4 seconds) then it will display "Stop Recording."

Now press enter to see the results.(same results will be available in out.txt file)

Now go to the directory YesNoMyCode--->YesNoMyCode.

There you can find your spoken speech in .wav as well as .txt format with the names as mentioned in "config.h" file.

scaled input speech samples can be found in "ScaledInpSpeech.txt"

Output result can be found in "out.txt"
