#pragma once


#include "include/vamp-hostsdk/PluginHostAdapter.h"
#include "include/vamp-hostsdk/PluginInputDomainAdapter.h"
#include "include/vamp-hostsdk/PluginLoader.h"
#include "include\sndfile.h"
#include "include\sndfile.hh"
#include <algorithm>
#include <fstream>
#include <map>

using namespace std;
using namespace Vamp;
using namespace Vamp::HostExt;

#define CHORDINO "nnls-chroma:chordino"
#define TEMPO "qm-vamp-plugins:qm-barbeattracker"
#define BEATROOT "beatroot-vamp:beatroot"
#define BARODD 3
#define BAREVEN 4

namespace Helper{
	static const float rate = 44100.00f;
	void appendFeatures(Plugin::FeatureSet &a, const Plugin::FeatureSet &b);
	bool inRange(float x, float left, float right);
	void output(bool even, string outputfile, map<float, pair<string, float>> chords, map<float, pair<string, float>> &bars, float avg_tempo);
	int generate(string file);
	void destroyTestAudio(float **b, size_t channels);
	float** createTestAudio(size_t channels, size_t blocksize, size_t blocks);
	float** loadWaveAudio_mono(string filepath, size_t &channels, size_t blocksize, size_t step, size_t &blocks);
	int process(string wav_file, map<float, pair<string, float>> &output_chords, map<float, pair<string, float>> &output_barseven, map<float, pair<string, float >> &output_barsodd, float &avg_tempo);
}