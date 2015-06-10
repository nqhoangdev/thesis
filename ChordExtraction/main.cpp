// ChordExtraction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Helper.h"
#include <algorithm>
#include <fstream>

using namespace std;
using namespace Vamp;
using namespace Vamp::HostExt;

int main(){
	Helper::generate("sample/bearlin.wav");
}