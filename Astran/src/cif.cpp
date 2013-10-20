
#include "cif.h"

Cif::Cif(string fname, Rules &actual_rules) {
	filename=fname;
	rules=&actual_rules;
	idx = 1;
	
	file.open(filename.c_str()); // Write
	cout << "Saving CIF file: " << fname << endl;
	if (!file)
		cerr << filename << ": CIF file could not be created" << endl;
	
	printHeader(file, "(", ");");
	file << "(CIF generated by the ICPD " << VERSION << ");\n";
	file << "(Technology : " << rules->getCIFVal(TECHNAME) << ");\n";

	instances.clear();
}

bool Cif::cellCif(map<string, CLayout>& layouts, string top) {
	if(layouts.find(top)==layouts.end())
		return false;

	CLayout& currentLayout=layouts[top];
	instances[currentLayout.getName()]=idx++;

	//Declara os Layouts utilizado no topo
	map<string, Instance>::iterator instances_it;
	for(instances_it=currentLayout.getInstances()->begin(); instances_it!=currentLayout.getInstances()->end(); instances_it++){
		if(instances.find(instances_it->second.getTargetCell())==instances.end())
			if(!cellCif(layouts, instances_it->second.getTargetCell()))
				return false;
	}

	//Instancia o layout atual
	file << "\nDS " << instances[currentLayout.getName()] << " 1 1;\n";
	
	file << "9 " << currentLayout.getName() << ";" << endl;
	
	//Chama as instancias
	for(instances_it=currentLayout.getInstances()->begin(); instances_it!=currentLayout.getInstances()->end(); instances_it++){
		file << "C " << instances[instances_it->second.getTargetCell()] << " ";
		if(instances_it->second.getMX()) file << "MX ";
		if(instances_it->second.getMY()) file << "MY ";
		file << "T " << instances_it->second.getX() << " " << instances_it->second.getY() << ";\n";
	}
	
	//Desenha o layout atual
	map <layer_name , list<Box> >::iterator layers_it; // iterador das camadas
	list<Label>::iterator labels_it;
	for (labels_it = currentLayout.labels.begin(); labels_it != currentLayout.labels.end(); labels_it++)
		file << "94 " << labels_it->text << " " << labels_it->pt.getX() << " "
		<< labels_it->pt.getY() << " " << rules->getCIFVal(MET1) << ";\n";
	
	list <Box>::iterator layer_it;
	for (layers_it = currentLayout.layers.begin(); layers_it != currentLayout.layers.end(); layers_it++) {
		if ( !layers_it->second.empty() ) {
			file << "L " << rules->getCIFVal(layers_it->first) << ";\n";
			for ( layer_it = layers_it->second.begin(); layer_it != layers_it->second.end(); layer_it++ )
				if ( layer_it->getHeight() > 0 && layer_it->getWidth() > 0 )
				file << "B " <<  2*layer_it->getWidth()  << " " <<
				2*layer_it->getHeight() << " " <<
                    2*layer_it->getX()+(layer_it->getWidth()%2?1:0) << " " <<
				2*layer_it->getY()+(layer_it->getHeight()%2?1:0) << ";\n";
			
		}
	}
	file << "DF;\n";
	return true;
}

bool Cif::cif2Cadence(string designName, string top){
	rules->saveCIFLayerTable(getPath(filename)+"CIFLTable.txt");
	
	ofstream c2cfile;
	string c2cfilename=removeExt(filename)+".c2c";
	c2cfile.open(c2cfilename.c_str()); // Write
	cout << "Saving CIF2Cadence convertion file: " << c2cfilename << endl;
	if (!c2cfile)
		cerr << c2cfilename << ": C2C file could not be created" << endl;
	c2cfile << "  cifInKeys = list( nil  " << endl;
	c2cfile << "    'runDir                 \"./\"" << endl;
	c2cfile << "    'inFile                 \"" << getFileName(filename) << "\"" << endl;
	c2cfile << "    'primaryCell            \"" << top << "\"" << endl;
	c2cfile << "    'libName                \"" << designName << "\"" << endl;
	c2cfile << "    'dataDump               \"\"" << endl;
	c2cfile << "    'techfileName           \"tech.lib\"" << endl;
	c2cfile << "    'scale                  " << 0.5/rules->getScale() << endl;
	c2cfile << "    'units                  \"micron\""   << endl;
	c2cfile << "    'errFile                \"PIPO.LOG\"" << endl;
	c2cfile << "    'cellMapTable           \"\"" << endl;
	c2cfile << "    'layerTable             \"CIFLTable.txt\"" << endl;
	c2cfile << "    'textFontTable          \"\"" << endl;
	c2cfile << "    'userSkillFile          \"\"" << endl;
	c2cfile << "    'hierDepth              32" << endl;
	c2cfile << "    'maxVertices            1024" << endl;
	c2cfile << "    'checkPolygon           nil" << endl;
	c2cfile << "    'snapToGrid             nil" << endl;
	c2cfile << "    'caseSensitivity        \"lower\"" << endl;
	c2cfile << "    'zeroWireToLine         \"lines\"" << endl;
	c2cfile << "    'wireToPathStyle        \"extend\"" << endl;
	c2cfile << "    'roundFlashToEllipse    \"ellipse\"" << endl;
	c2cfile << "    'skipUndefinedLPP       nil" << endl;
	c2cfile << "    'reportPrecision        nil" << endl;
	c2cfile << "    'runQuiet               nil" << endl;
	c2cfile << "    'saveAtTheEnd           nil" << endl;
	c2cfile << "    'noWriteExistCell               nil" << endl;
	c2cfile << "    'NOUnmappingLayerWarning                nil" << endl;
	c2cfile << "    )" << endl;
	c2cfile.close();
	cout << "ATTENTION:" << endl;
	cout << "Copy the files: " << getFileName(c2cfilename) << ", " << getFileName(filename) << ", CIFLTable.txt, tech.lib (dump it direct from Cadence Icfb: Tools->Technology File Manager->Dump) to the root of your cadence project and than execute in the icfb menu File->Import->CIF Load: " << getFileName(c2cfilename) << endl << "After that, execute Tools->Technology File Manager->Attach, select your Design Library (usually new_design) and the target Technology Library to Attach (e.g. cmos130), you only need to do it once. Now you are ready to use Cadence Virtuoso." << endl;
	return true;
}

Cif::~Cif(){
	//	file << "C 1;\n";
		file << "E;\n";
		file.flush();
		file.close();
}