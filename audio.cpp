#include <cstring>
#include <stdlib.h>
#include <stdio.h>	
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512


using namespace std;

static void checkError(PaError err) {
	if (err != paNoError) {
		printf("PortAudio error: %s\n", Pa_GetErrorText(err));
		exit(1);
	}
}

static inline float absloute(float a){
	if (a > 0) {
		return a;
	}
	return -a;
}

static inline float max(float a,float b){
	if ( a > b){
		return a;
	} 
	return b;
}

static int pastestCallback (const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
	float* in = (float*) inputBuffer;
	(void)outputBuffer;
	int dispSize = 100;
	printf("\r");


	float vol_1 = 0;
	float vol_2 = 0;

	for (unsigned long i = 0; i < framesPerBuffer *2; i +=2){
		vol_1 = max(vol_1,absloute(in[i]));
		vol_2 = (max(vol_2, absloute(in[i+1])));
	}
	for (int i= 0 ;i <dispSize;i++){
		float barProportion = i / (float)dispSize;
		if (barProportion <=vol_1 && barProportion<=vol_2){
			printf("█");//full block
		}else if (barProportion <=vol_1){
			printf("▀");//top half
		}else if (barProportion <=vol_2) {
			printf("▄");// bottom half
		}else {
			printf(" ");
		}
	}

	fflush(stdout);
		return 0;
}


int main () {
	PaError err;
	err= Pa_Initialize();
	checkError(err);

	int numDevices = Pa_GetDeviceCount();
	printf("Number of devices: %d\n", numDevices);

	if (numDevices < 0) {
		printf("Error getting device count\n");
		return 1;
	}else if (numDevices == 0) {
		printf("No device found\n");
	}



const PaDeviceInfo* deviceInfo; 


	for (int i = 0; i < numDevices; i++) {
	    deviceInfo	= Pa_GetDeviceInfo(i);
		printf("     Device %d:\n", i);
		printf("name: %s\n", deviceInfo->name);
		printf("maxInputChannels %d\n", deviceInfo->maxInputChannels);
		printf("maxOutputChannels %d\n",  deviceInfo->maxOutputChannels);
		printf("defaultSampleRate %f\n",  deviceInfo->defaultSampleRate);
	}

	int device = 5;
	PaStreamParameters inputParameters;
	PaStreamParameters outputParameters;

	
	memset(&inputParameters, 0, sizeof(inputParameters));
	inputParameters.channelCount = 2;
	inputParameters.device = device;
	inputParameters.hostApiSpecificStreamInfo = NULL;
	inputParameters.sampleFormat = paFloat32;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;

	
	memset(&outputParameters, 0, sizeof(outputParameters));
	outputParameters.channelCount = 2;
	outputParameters.device = device;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;


	PaStream* stream;
	err = Pa_OpenStream(&stream, &inputParameters, &outputParameters, SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag,pastestCallback , NULL);
	checkError(err);

	err = Pa_StartStream(stream);
	checkError(err);

	Pa_Sleep(10 * 1000);

	err=Pa_StopStream(stream);
	checkError(err);

	err=Pa_CloseStream(stream);
	checkError(err);


	err = Pa_Terminate();
	checkError(err);


	return 0;

}
