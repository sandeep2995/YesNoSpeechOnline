#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h> 
#include <conio.h>
#include <sstream>
#include "config.h" //include configuration file that helps in global settings

using namespace std;
std::ostringstream oss;

short samples[600000], MaxAmp=0; //Maximum number of samples we can handle and Maximum amplitude desired for scaling
float ThresholdZCR, DCshift=0; //Initialize DCShift and Threshold ZCR
double TotalEnergy=0, ThresholdEnergy, NoiseEnergy=0, TotalZCR=0; //TotalZCR is only for first few InitFrames
long start=0, stop=0, framecount, samplecount=0; //start and end marker for speech signal YES or NO
string line, filename;

ifstream InpSpeech;
ofstream ScaledInpSpeech,out;

int main()
{
	long i,j;

	oss << recmod << " "<<duration <<" "<<inpw << " " << inpt; //define the string manually, because macro cant expand inside string
	system(oss.str().c_str()); //call the system function to read directly from the mic
	filename=inpt; //name of the recorded speech text file

	InpSpeech.open(filename, ios::in); // open the file to read from it
	if (!InpSpeech) //if file is not present pop the error
	{
		cout << "\n**File failed to open**\n\n";
		InpSpeech.clear();
	}

	out.open("out.txt"); // open the file to write the results
			     //count the number of samples and frames in the file
	if (InpSpeech.is_open()) //if file is open then only execute this block
	{
		while ( !InpSpeech.eof() ) //read till the end of file is reached
		{ 
			getline (InpSpeech,line); //read a line at a time
			samplecount+=1;     //increment the sample count by 1
			if(samplecount > IgnoreSamples + 4) //first 4 lines in text file indiactes type of encoding of the speech,
				{
					samples[samplecount - (IgnoreSamples + 5)] = (short)atoi(line.c_str()); //5=4+1 4-->indicates encoding, 1-->array index starts with 0
					DCshift+=samples[samplecount - (IgnoreSamples + 5)];
					if(abs(samples[samplecount - (IgnoreSamples + 5)])>MaxAmp)
						MaxAmp=abs(samples[samplecount - (IgnoreSamples + 5)]); //MaxAmp contains the magnitude od the maximum valued(absolute) sample
				}
		}
		InpSpeech.close();
	}
	samplecount = samplecount - (IgnoreSamples + 4);
	framecount = samplecount/framesize;
	DCshift=DCshift/samplecount;
	out << "Number of Samples = " << samplecount << "\n"; // print the results in out.txt file
	out << "Number of frames = " << framecount << "\n";
	out << "DC shift needed is " << DCshift << "\n";
	out << "Maximum Amplitude = " << MaxAmp << "\n";
	cout << "Number of Samples = " << samplecount << "\n"; // print the results on standard output
	cout << "Number of frames = " << framecount << "\n";
	cout << "DC shift needed is " << DCshift << "\n";
	cout << "Maximum Amplitude = " << MaxAmp << "\n";

	//Store scaled samples in different file
	ScaledInpSpeech.open("ScaledInpSpeech.txt"); // open ScaledInpSpeech.txt to write the scaled sample values
	for(i=0;i<samplecount;i++)
		ScaledInpSpeech << (samples[i] - DCshift)*Ampscale/MaxAmp << "\n"; // writing the scaled samples to the file ScaledInpSpeech.txt
	ScaledInpSpeech.close();
	//use the scaled samples
	InpSpeech.open("ScaledInpSpeech.txt", ios::in); // open ScaledInpSpeech.txt to read the scaled samples
	if (!InpSpeech)
	{
		cout << "\n**File failed to open**\n\n";
		InpSpeech.clear();
	}
	InpSpeech.close(); // close the file

	//ZCR and Energy calculation
	float AvgZCR[400], AvgEnergy[400];
	for(i=0;i<framecount;i++)
	{
		AvgZCR[i]=0; // Initialize Average ZCR for each frame
		AvgEnergy[i]=0; // Initialize Average Energy for each frame
		for(j=0;j<framesize-1;j++)
		{
			if((samples[i*framesize + j]-DCshift)*Ampscale/MaxAmp*((samples[i*framesize + j + 1]-DCshift)*Ampscale*1.0 < 0)) // If two adjacent samples have opposite sign then increment the avergae ZCR by 1
				AvgZCR[i]+=1;
			AvgEnergy[i]+=1.0*(samples[i*framesize + j]-DCshift)*Ampscale/MaxAmp*(samples[i*framesize + j]-DCshift)*Ampscale/MaxAmp; // energy is calculated as square of amplitude of the sample value
		}
		//out << "ZCR in "<<i+1<<"th frame is "<< AvgZCR[i] << "\n";
		AvgZCR[i]/=framesize;
		//out << "Avergae ZCR in\t\t "<<i+1<<"th frame is "<< AvgZCR[i] << "\n";
		AvgEnergy[i]+=1.0*(samples[i*framesize + j]-DCshift)*Ampscale/MaxAmp*(samples[i*framesize + j]-DCshift)*Ampscale/MaxAmp; // average energy for all the frames after scaled sample values
																	 //out << "Energy in\t "<<i+1<<"th frame is "<< AvgEnergy[i] << "\n";
		AvgEnergy[i]/=framesize; // average energy per frame after scaled sample values 
					 //out << "Average Energy in \t\t\t"<<i+1<<"th frame is "<< AvgEnergy[i] << "\n";
		TotalEnergy+=AvgEnergy[i];
	}
	//calculate noise energy to decide threshold for energy
	for(i=0;i<InitFrames;i++)
	{
		TotalZCR+=AvgZCR[i];
		NoiseEnergy+=AvgEnergy[i];
	}
	ThresholdZCR=TotalZCR/InitFrames;
	NoiseEnergy/=InitFrames;
	ThresholdZCR*=0.9;
	//ThresholdEnergy=TotalEnergy/framecount; //threshold energy is the average of all averaged energies
	//ThresholdEnergy*=0.9;
	ThresholdEnergy=NoiseEnergy*10;
	bool flag=false;
	//start and end marker of speech
	for(i=0;i<framecount-3;i++)
	{
		//if(AvgEnergy[i+1]>ThresholdEnergy)
		if(AvgZCR[i+1]<ThresholdZCR || AvgEnergy[i+1]>ThresholdEnergy)
		{
			if(flag == false && AvgEnergy[i+2]>ThresholdEnergy && AvgEnergy[i+3]>ThresholdEnergy)
			{
				start = i  ; //i th frame is the starting frame marker
				out << "Starting frame is "<< start+1 <<"th frame and starting sample is "<< (start+1)*framesize <<"\n"; // write starting frame and sample number in out.txt file
				out << "Starting time = " << 1.0*(start + 1)*framesize/samplingrate << " seconds\n" ;// writing starting sample time in seconds to out.txt
				cout << "Starting frame is "<< start+1 <<"th frame and starting sample is "<< (start+1)*framesize <<"\n"; // write starting frame and sample number on standard output
				cout << "Starting time = " << 1.0*(start + 1)*framesize/samplingrate << " seconds\n" ; // writing starting sample time in seconds to standard output
				flag = true;
			}
		}
		else if(flag == true && AvgZCR[i] > ThresholdZCR && AvgEnergy[i] < ThresholdEnergy && AvgEnergy[i-1] < ThresholdEnergy && AvgEnergy[i-2] < ThresholdEnergy)
		{
			stop = i ;
			out << "Ending frame is "<< stop+1 <<"th frame and Ending sample is "<< (stop+1)*framesize<<"\n"; // write ending frame and sample number in out.txt file
			out << "Ending time = " << 1.0*(stop + 1)*framesize/samplingrate << " seconds\n" ; // writing ending sample time in seconds to out.txt
			cout << "Ending frame is "<< stop+1 <<"th frame and Ending sample is "<< (stop+1)*framesize<<"\n"; // write ending frame and sample number on standard output
			cout << "Ending time = " << 1.0*(stop + 1)*framesize/samplingrate << " seconds\n" ; // writing ending sample time in seconds to standard output
			flag = false;
			break;
		}
	}

	double testavgE=0, testavgZ=0;
	for(i=start;i<=stop;i++)   
	{
		testavgE+=AvgEnergy[i]; // sum the avergae energy from start to stop frame to compute over all avegare energy.
		testavgZ+=AvgZCR[i]; // sum the avergae zcr from start to stop frame to compute over all avegare ZCR. 
	}
	testavgE/=(stop-start+1); // average energy per sample
	testavgZ/=(stop-start+1); // average ZCR per sample
	out << "Average Energy of the speech signal is "<< testavgE << "\n";  // write average energy to file out.txt
	out << "Average ZCR of the speech signal is "<< testavgZ*framesize << " \n"; // write average energy to standard output
	cout << "Average Energy of the speech signal is "<< testavgE << "\n"; //write average energy to standard output
	cout << "Average ZCR of the speech signal is "<< testavgZ*framesize << " \n"; // write average ZCR to standard output

	float lcount=0, rcount=0;
	for(i=stop;i>=start;i--)
	{
		if(AvgZCR[i]>testavgZ && AvgEnergy[i] < testavgE)
			rcount++;  // count the number of frames on right side of the speech signal based on average ZCR and average energy.
		if(AvgZCR[start+stop-i]< testavgZ && AvgEnergy[i] < testavgE)
			lcount++; // count the number of frames on left side of the speech signal based on average ZCR and average energy.
	}


	long mark;
	mark=(stop+start)/2;

	if(mark == 0) //If there are no starting and end markers then it is a non speech signal
	{
		out << "Non speech signal\n";
		cout << "Non speech signal\n";
	}

	else if(rcount > 10*300/framesize) //check condition for YES speech signal
	{
		out << "speech is YES signal\n";
		cout << "speech is YES signal\n";
	}

	else //If it is not non speech and not a YES signal then it must be a NO signal(for this assignment)
	{
		out << "speech is NO signal\n\n";
		cout << "speech is NO signal\n\n";
	}
	cout << "NOTE: To see the speech samples, check ScaledInpSpeech.txt file \n";
	out.close();
	cout << "Check the output in out.txt file\n"; 
	getch();
	return 0;
}
