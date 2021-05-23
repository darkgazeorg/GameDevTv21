#include <Gorgon/Main.h>
#include <Gorgon/Window.h>
#include <Gorgon/Geometry/Point.h>
#include <Gorgon/Graphics/Bitmap.h>
#include <Gorgon/Graphics/Layer.h>
#include <Gorgon/Graphics/FreeType.h>
#include <Gorgon/String.h>
#include <Gorgon/Filesystem.h>
#include <Gorgon/Filesystem/Iterator.h>
#include <Gorgon/Resource/File.h>
#include <Gorgon/Resource/Image.h>
#include <Gorgon/Resource/Data.h>

#include <functional>
#include <fstream>
#include <iostream>

using namespace Gorgon::Graphics;
namespace String = Gorgon::String;
namespace FS = Gorgon::Filesystem;
namespace R = Gorgon::Resource;
using Gorgon::Geometry::Point;

Bitmap Scale(const Bitmap &bmp, int scale) {
	Bitmap n(bmp.GetSize() * scale, bmp.GetMode());

	bmp.ForAllPixels([&bmp, &n, scale](int x, int y, int c) {
		for(int yy=0; yy<scale; yy++)
			for(int xx=0; xx<scale; xx++)
				n(x*scale + xx, y*scale + yy, c) = bmp(x, y, c);
	});

	return n;
}

void doimport(int scale) {
	int count = 0;

	struct fold {
		std::string fold_name;
		std::vector<std::string> filenames;
	};

	std::vector<fold> folders;

	for(FS::Iterator it("../assets/"); it.IsValid(); it.Next()) {
		auto fold_name = *it;
		auto fold_path = FS::Join("../assets/", fold_name);

		//ignore files
		if(!FS::IsDirectory(fold_path))
			continue;

		folders.push_back({fold_name, {}});

		for(FS::Iterator it(fold_path); it.IsValid(); it.Next()) {
			auto file_name = *it;
			auto file_path = FS::Join(fold_path, file_name);

			if(Gorgon::String::ToLower(FS::GetExtension(file_name)) != "png")
				continue;

			Bitmap im;
			if(im.Import(file_path)) {
				folders.back().filenames.push_back(file_name);
				count++;
			}
		}
	}


	R::File file;
	int ind = 1;

	std::sort(folders.begin(), folders.end(), [](const fold &l, const fold &r) {
		return l.fold_name < r.fold_name;
	});
	for(auto &f : folders) {
		auto fold_name = f.fold_name;
		auto fold_path = FS::Join("../assets/", fold_name);

		std::cout << String::Concat("Importing folder ", fold_name, "...") << std::endl;

		//ignore files
		if(!FS::IsDirectory(fold_path))
			continue;

		auto &fold = *new R::Folder();
		file.Root().Add(fold);
		fold.SetName(fold_name);

		std::sort(f.filenames.begin(), f.filenames.end());

		for(auto &fn : f.filenames) {
			auto file_name = fn;
			auto file_path = FS::Join(fold_path, file_name);
            
			Bitmap im;
			if(im.Import(file_path)) {
				im = Scale(im, scale);
                
				auto &imres = *new R::Image(std::move(im));
				fold.Add(imres);
                
                
                if(fold_name == "Towers"){
                
                    auto &data  = *new R::Data;
                    fold.Add(data);

                    auto name = FS::GetBasename(file_name);
                    int tid = 0; //Tower ID
                    int dpb = 0; //damage per bullet
                    float reloadt = 0.0; //reload time
                    int bcapacity = 0; //bullet capacity
                    int cr = 0; //Continuious reload
                    float bspeed = 0.0;
                    int damagetype = 0;
                    float areasize = 0.0;
                    float damagefalloff = 0.0;
                    float damagefalloffarea = 0;
                    int cost = 0;
                    std::string upgradepath = "";
                    float range = 0.0;
                    int placeable = 0;
                    int target = 0;
                    float ea = 0.0; //effective against
                    float em = 0.0; //effective multiplier
                    float bacc = 0.0; //bullet acceleration
                    auto startlocation = Point(0, 0);
                    //auto off  = ;

                    std::ifstream dataf(FS::Join(fold_path, name + ".txt"));

                    if(dataf.is_open()) {
                        std::string line;
                        
                        std::cout << "Found txt file for data" << std::endl;

                        std::getline(dataf, line);
                        if(!line.empty())
                            tid = String::To<int>(line);
                        
                        std::cout << "ID = "<< std::to_string(tid) << std::endl;

                        std::getline(dataf, line);
                        if(!line.empty())
                            dpb = String::To<int>(line);
                        
                        std::getline(dataf, line);
                        if(!line.empty())
                            reloadt = String::To<float>(line);
                            
                        std::getline(dataf, line);
                        if(!line.empty())
                            bcapacity = String::To<int>(line);
                            
                        std::getline(dataf, line);
                        if(!line.empty())
                            cr = String::To<int>(line);
                            
                        std::getline(dataf, line);
                        if(!line.empty())
                            bspeed = String::To<float>(line);
                            
                        std::getline(dataf, line);
                        if(!line.empty())
                            damagetype = String::To<int>(line);
                            
                        std::getline(dataf, line);
                        if(!line.empty())
                            areasize = String::To<float>(line);
                            
                        std::getline(dataf, line);
                        if(!line.empty())
                            damagefalloff = String::To<float>(line);
                            
                        std::getline(dataf, line);
                        if(!line.empty())
                            damagefalloffarea = String::To<float>(line);
                        
                        std::getline(dataf, line);
                        if(!line.empty())
                            cost = String::To<int>(line);
                        
                        std::getline(dataf, line);
                        if(!line.empty())
                            upgradepath = line;
                        
                        std::getline(dataf, line);
                        if(!line.empty())
                            range = String::To<float>(line);
                        
                        std::getline(dataf, line);
                        if(!line.empty())
                            placeable = String::To<int>(line);
                        
                        std::getline(dataf, line);
                        if(!line.empty())
                            target = String::To<int>(line);
                        
                        std::getline(dataf, line);
                        if(!line.empty())
                            ea = String::To<float>(line);
                        
                        std::getline(dataf, line);
                        if(!line.empty())
                            em = String::To<float>(line);
                        
                        std::getline(dataf, line);
                        if(!line.empty())
                            bacc = String::To<float>(line);
                        
                        std::getline(dataf, line);
                        if(!line.empty())
                            startlocation  = String::To<Point>(line);
                    }
                    
                    data.Append("tid", tid);
                    data.Append("dpb", dpb);
                    data.Append("reloadt", reloadt);
                    data.Append("bcapacity", bcapacity);
                    data.Append("cr", cr);
                    data.Append("bspeed", bspeed);
                    data.Append("damagetype", damagetype);
                    data.Append("areasize", areasize);
                    data.Append("damagefalloff", damagefalloff);
                    data.Append("damagefalloffarea", damagefalloffarea);
                    data.Append("cost", cost);
                    data.Append("upgradepath", upgradepath);
                    data.Append("range", range);
                    data.Append("placeable", placeable);
                    data.Append("target", target);
                    data.Append("ea", ea);
                    data.Append("em", em);
                    data.Append("bacc", bacc);
                    data.Append("startlocation", startlocation);
                }

				std::cout << String::Concat("Imported ", file_name, ".") << std::endl;
				ind++;
			}
			else {
				std::cout << String::Concat("Cannot import file: ", file_name, "!") << std::endl;
			}
		}
	}

	file.Save(String::Concat("../../bin/","Resources_", scale, "x.gor"));
    std::cout << "All files Imported!" << std::endl;
}

// if commandline contains a number, the operation will be performed silently
int main(int argc, char *argv[]) {

    try {
        doimport(1);
    }
    catch(const std::runtime_error &ex) {
        std::cerr<<ex.what()<<std::endl;

        return -1;
    }


	return 0;
}
