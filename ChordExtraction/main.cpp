// ChordExtraction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Helper.h"
#include <algorithm>
#include <fstream>

using namespace std;
using namespace Vamp;
using namespace Vamp::HostExt;

float threshold = 0.0f;
bool inRange(float x, float left, float right){
	return x >= left && x <= right;
}

void output(bool even, string outputfile, map<float, pair<string, float>> chords, map<float, pair<string, float>> &bars, float avg_tempo){
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
int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR* input = argv[1],* input2 = argv[2];
	
	string file = input;
	string outfile = input2;
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
