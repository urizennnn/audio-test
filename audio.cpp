#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>	
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
const int NUM_CHANNELS = 2;

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

static int recordAudio (const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
	std::vector<short>* recordedSamples = (std::vector<short>*)userData;
    const short* rptr = (const short*)inputBuffer;
    unsigned long framesLeft = framesPerBuffer;
    size_t maxSamples = recordedSamples->size() + framesPerBuffer;
    recordedSamples->reserve(maxSamples);

    while (framesLeft > 0) {
        recordedSamples->push_back(*rptr);
        rptr++;
        framesLeft--;
    }

    return paContinue;
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
	vector<short> recorded;
	err = Pa_OpenStream(&stream, &inputParameters, &outputParameters, SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag,recordAudio , &recorded);
	checkError(err);

	err = Pa_StartStream(stream);
	checkError(err);

	cout << "Recording Audio... Click anywhere to stop" << endl;
	cin.get();

	err=Pa_StopStream(stream);
	checkError(err);

	ofstream wavFile("recorded_audio.wav",ios::binary);
	checkError(err);

	const int numSamples = static_cast<int>(recorded.size());
    const int numChannels = NUM_CHANNELS;
    const int bytesPerSample = 2; // 16 bits per sample
    const int byteRate = SAMPLE_RATE * numChannels * bytesPerSample;
    const int blockAlign = numChannels * bytesPerSample;
    const int dataChunkSize = numSamples * numChannels * bytesPerSample;



uint32_t temp32;
uint16_t temp16;

wavFile.write("RIFF", 4);

temp32 = static_cast<uint32_t>(dataChunkSize + 36);
wavFile.write(reinterpret_cast<const char*>(&temp32), sizeof(uint32_t));

wavFile.write("WAVE", 4);
wavFile.write("fmt ", 4);

temp32 = 16;
wavFile.write(reinterpret_cast<const char*>(&temp32), sizeof(uint32_t));

temp16 = 1; // PCM
wavFile.write(reinterpret_cast<const char*>(&temp16), sizeof(uint16_t));

temp16 = numChannels;
wavFile.write(reinterpret_cast<const char*>(&temp16), sizeof(uint16_t));

temp32 = SAMPLE_RATE;
wavFile.write(reinterpret_cast<const char*>(&temp32), sizeof(uint32_t));

temp32 = byteRate;
wavFile.write(reinterpret_cast<const char*>(&temp32), sizeof(uint32_t));

temp16 = blockAlign;
wavFile.write(reinterpret_cast<const char*>(&temp16), sizeof(uint16_t));

temp16 = 16; // Bits per sample
wavFile.write(reinterpret_cast<const char*>(&temp16), sizeof(uint16_t));

wavFile.write("data", 4);

temp32 = dataChunkSize;
wavFile.write(reinterpret_cast<const char*>(&temp32), sizeof(uint32_t));

// Write the audio data
wavFile.write(reinterpret_cast<const char*>(recorded.data()), dataChunkSize);

wavFile.close();



	err=Pa_CloseStream(stream);
	checkError(err);


	err = Pa_Terminate();
	checkError(err);


	return 0;

}
