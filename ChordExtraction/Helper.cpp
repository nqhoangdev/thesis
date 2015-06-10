#include "Helper.h"

using namespace Helper;
bool Helper::inRange(float x, float left, float right){
	return x >= left && x <= right;
}

void Helper::output(bool even, string outputfile, map<float, pair<string, float>> chords, map<float, pair<string, float>> &bars, float avg_tempo){
	auto bar = bars.begin();
	ofstream os;
	os.open(outputfile.c_str());
	if (even)
		os << "(meter 4 4)" << endl;
	else
		os << "(meter 3 4)" << endl;
	os << "(tempo " << avg_tempo << ")" << endl;
	os << "(section (style two-beat))\n";
	string previousChord = "null";
	//
	do {
		auto chord = chords.begin();
		string str = "";
		while (chord != chords.end())
		{
			// Check intersect between a bar and a chord
			float barbegin = bar->first;
			float barend = bar->first + bar->second.second;
			float chordbegin = chord->first;
			float chordend = chord->first + chord->second.second;
			if (inRange(barbegin, chordbegin, chordend) || inRange(barend, chordbegin, chordend)){
				float intersect = min(chordend, barend) - max(chordbegin, barbegin);
				float ratio = intersect / bar->second.second;
				if (chord->second.first != "N"){
					string currentChord = chord->second.first;
					printf("%s ", currentChord.c_str());
					if (currentChord == previousChord){
						str += " / ";
					}
					else {
						str = " " + currentChord + " ";
						previousChord = currentChord;
					}
				}
			}
			++chord;
			//
		}
		if (str.length() > 0) {
			bar->second.first = str;
			os << bar->second.first << " | ";
		}
		++bar;
	} while (bar != bars.end());
	os.close();
}
int Helper::generate(string file){
	//string p
	//map: begin time, label, duration
	map<float, pair<string, float>> chords, bars, barseven, barsodd;
	float avg_tempo;
	process(file, chords, barseven, barsodd, avg_tempo);
	output(true, "even.ls", chords, barseven, avg_tempo);
	output(false, "odd.ls", chords, barsodd, avg_tempo);


	auto bar = barseven.begin();
	float begin = bar->first;
	string strbegin = to_string(begin);
	string cmd("java -jar genAccompaniment.jar even.ls even.mid ");
	cmd += strbegin + " ";
	cmd += file;
	system(cmd.c_str());
	return 0;
}


void Helper::appendFeatures(Plugin::FeatureSet &a, const Plugin::FeatureSet &b)
{
	for (Plugin::FeatureSet::const_iterator i = b.begin(); i != b.end(); ++i) {
		int output = i->first;
		const Plugin::FeatureList &fl = i->second;
		Plugin::FeatureList &target = a[output];
		for (Plugin::FeatureList::const_iterator j = fl.begin(); j != fl.end(); ++j) {
			target.push_back(*j);
		}
	}
}
void Helper::destroyTestAudio(float **b, size_t channels)
{
	for (size_t c = 0; c < channels; ++c) {
		delete[] b[c];
	}
	delete[] b;
}

float** Helper::createTestAudio(size_t channels, size_t blocksize, size_t blocks)
{
	float **b = new float *[channels];
	for (size_t c = 0; c < channels; ++c) {
		b[c] = new float[blocksize * blocks];
		for (int i = 0; i < int(blocksize * blocks); ++i) {
			b[c][i] = sinf(float(i) / 10.f);
			if (i == 5005 || i == 20002) {
				b[c][i - 2] = 0;
				b[c][i - 1] = -1;
				b[c][i] = 1;
			}
		}
	}
	return b;
}

float** Helper::loadWaveAudio_mono(string filepath, size_t &channels, size_t blocksize, size_t step, size_t &blocks){
	SndfileHandle file;
	file = SndfileHandle(filepath);
	channels = 1;
	blocks = ceil(file.frames() / step);
	int buffersz = file.frames();
	float** buffer = new float*[channels];
	buffer[0] = new float[buffersz];
	file.readf(buffer[0], buffersz);

	return buffer;
}


int Helper::process(string wav_file, map<float, pair<string, float>> &output_chords, map<float, pair<string, float>> &output_barseven, map<float, pair<string, float >> &output_barsodd, float &avg_tempo){
	// Get names of all VAMP Plugins
	Vamp::HostExt::PluginLoader::PluginKeyList keys =
		Vamp::HostExt::PluginLoader::getInstance()->listPlugins();
	// Get Chordino plugin (nnls-chroma:chordino)
	Plugin *p = PluginLoader::getInstance()->loadPlugin
		(CHORDINO, rate, PluginLoader::ADAPT_ALL);
	Plugin *pBar = PluginLoader::getInstance()->loadPlugin
		(BEATROOT, rate, PluginLoader::ADAPT_ALL);
	if (!p)
		return -1;
	PluginInputDomainAdapter *pa = new PluginInputDomainAdapter(p);
	PluginInputDomainAdapter *paBar = new PluginInputDomainAdapter(pBar);
	//PluginInputDomainAdapter *paBarOdd = new PluginInputDomainAdapter(pBarOdd);
	// Features data
	
	//auto c = paBar->getOutputDescriptors();
	//auto c = pa->getParameterDescriptors();
	
	Plugin::FeatureSet f;
	Plugin::FeatureSet fBar;
	//Plugin::FeatureSet fBarOdd;

	//Results r;
	float **data = 0;
	size_t channels = pa->getMaxChannelCount();
	size_t step = 4096;// pa->getPreferredStepSize();
	//size_t step = pa->getPreferredBlockSize();
	size_t block = 4096; pa->getPreferredBlockSize();
	size_t count = 100; //= number of blocks

	//paBarOdd->setParameter("bpb", 3);
	// Init plugin

	//pa->initialise(channels, step, block);
	pa->initialise(channels, step, block);
	paBar->initialise(channels, step, block);
	//paBarOdd->initialise(channels, step, block);

	bool real = true;
	// Option 1:Create a test audio
	if (!real)
		data = createTestAudio(channels, block, count);
	else
		data = loadWaveAudio_mono(wav_file, channels, block, step, count);
	// Option 2: Load wave file
	// Run the plugin

	for (size_t i = 0; i < count; ++i) {

		float **ptr = (float **)alloca(channels * sizeof(float));
		size_t idx = i * step;
		for (size_t c = 0; c < channels; ++c) ptr[c] = data[c] + idx;
		RealTime timestamp = RealTime::frame2RealTime(idx, rate);
		Plugin::FeatureSet fs = pa->process(ptr, timestamp);
		Plugin::FeatureSet fsTempo = paBar->process(ptr, timestamp);
		//Plugin::FeatureSet fsBarOdd = paBarOdd->process(ptr, timestamp);
		appendFeatures(f, fs);
		appendFeatures(fBar, fsTempo);
		//appendFeatures(fBarOdd, fsTempo);
	}
	Plugin::FeatureSet fs = pa->getRemainingFeatures();
	Plugin::FeatureSet fsTempo = paBar->getRemainingFeatures();
	//Plugin::FeatureSet fsBarOdd = paBarOdd->getRemainingFeatures();
	appendFeatures(f, fs);
	appendFeatures(fBar, fsTempo);
	//appendFeatures(fBarOdd, fsBarOdd);
	//pa->g
	// Output Chords
	auto ft = f.at(0);
	int n = ft.size();
	for (int i = 0; i < n; ++i){
		float duration = 10.0f, t0, t1;
		Plugin::Feature feature = ft.at(i);
		t0 = feature.timestamp.sec + (float)feature.timestamp.msec() / 1000;
		//output_chords[t0] = feature.label;
		if (i < n - 1){
			auto feature2 = ft.at(i + 1);
			t1 = feature2.timestamp.sec + (float)feature2.timestamp.msec() / 1000;
			duration = t1 - t0;
		}
		output_chords[t0] = make_pair(feature.label, duration);
	}
	// Output Bar and Beat
	ft = fBar.at(0);
	n = ft.size();
	float t_odd = 0.0f, t_even = 0.0f;
	float durodd = 0.0f, dureven = 0.0f;
	for (int i = 0; i < n; ++i){
		float duration = 10.0f, t0, t1;
		Plugin::Feature feature = ft.at(i);
		t0 = feature.timestamp.sec + (float)feature.timestamp.msec() / 1000;
		//output_chords[t0] = feature.label;
		if (i < n - 1){
			auto feature2 = ft.at(i + 1);
			t1 = feature2.timestamp.sec + (float)feature2.timestamp.msec() / 1000;
			duration = t1 - t0;
		}
		if (i % BARODD == 0){
			t_odd = t0;
			durodd = duration;
			output_barsodd[t_odd] = make_pair(feature.label, durodd);
		}
		if (i % BAREVEN == 0){
			t_even = t0;
			dureven = duration;
			output_barseven[t_even] = make_pair(feature.label, dureven);
		}
		
		// Update
		durodd += duration;
		dureven += duration;
		//
		output_barsodd[t_odd].second = durodd;
		output_barseven[t_even].second = dureven;
	}

	// Avg Tempo
	avg_tempo = 0;
	ft = fBar.at(0);
	n = ft.size();
	for (int i = 0; i < n - 1; ++i){
		Plugin::Feature f0 = ft.at(i);
		Plugin::Feature f1 = ft.at(i+1);
		float t0 = f0.timestamp.sec + (float)f0.timestamp.msec() / 1000;
		float t1 = f1.timestamp.sec + (float)f1.timestamp.msec() / 1000;
		avg_tempo += 1.0f / (t1 - t0) * 60;
	}
	avg_tempo /= n;
	if (data) destroyTestAudio(data, channels);
}