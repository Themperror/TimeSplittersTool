#include "TS2Importer.h"
#include "TS2Texture.h"
#include "TS2Model.h"
#include "common.h"
#include "stringutils.h"
#include "fileutils.h"
#include "memoryreader.h"
#include "binarywriter.h"

#include <assert.h>

#include <algorithm>
#include <filesystem>
bool TS2Importer::PAKExtract(const std::string& pathToExtractedISO, const std::string& outputDirectory)
{
	std::string pakPath = Utility::ReplaceChar(pathToExtractedISO, '/', '\\');;
	if (!pakPath.ends_with('\\'))
		pakPath.append("\\");

	pakPath.append("pak");

	std::vector<std::string> files = Utility::GetFilesInDirectoryRecursive(pakPath);

	for (size_t i = files.size() - 1; i < files.size(); i--)
	{
		if (!files[i].ends_with(".pak"))
		{
			files.erase(files.begin() + i);
		}
	}

	pakFiles.reserve(files.size());
	for (size_t i = 0; i < files.size(); i++)
	{
		const auto& fileData = Utility::ReadFileToVector(files[i]);
		Utility::MemoryReader reader((char*)fileData.data(),fileData.size());
		PakFile pak;
		pak.pakHeader = reader.Read<PakFile::PakHeader>();

		switch (pak.GetPakVersion())
		{
			case PakFile::PakVersion::P4CK:
			pak.LoadEntriesV4(reader);
			break;
			case PakFile::PakVersion::P5CK:
			Utility::Print("Import for Timesplitters Future Perfect not supported yet!");
			Utility::Break();
			break;
			case PakFile::PakVersion::P8CK:
			Utility::Print("Import for Timesplitters 2 GC / Homefront not supported yet!");
			Utility::Break();
			break;
		}

		#if DEBUG
		Utility::Print("Pak File: %s\nVersion: %c%c%c%c\nNum Files: %i\n\nFiles:", files[i].c_str(), pak.pakHeader.magic[0], pak.pakHeader.magic[1], pak.pakHeader.magic[2], pak.pakHeader.magic[3], pak.fileEntries.size());
		for(const auto& it : pak.fileEntries)
		{
			Utility::Print("%s", it.first.c_str());
		}
		#endif
		pak.filename = files[i];
		pakFiles.push_back(pak);

		for (const auto& file : pak.fileEntries)
		{
			std::filesystem::create_directories(outputDirectory + "\\" + Utility::GetPathName(file.first));
			Utility::WriteFile(fileData.data() + file.second.offset, file.second.size, outputDirectory + "\\" + file.first);
		}
	}
	return true;
}

bool TS2Importer::TexConvert(const std::string& pathToExtractedData, const std::string& outputDirectory)
{
	std::string texPath = Utility::ReplaceChar(pathToExtractedData, '/', '\\');;
	if (!texPath.ends_with('\\'))
		texPath.append("\\");

	texPath.append("textures");
	std::vector<std::string> files = Utility::GetFilesInDirectoryRecursive(texPath);

	for (size_t i = files.size() - 1; i < files.size(); i--)
	{
		if (!files[i].ends_with(".raw"))
		{
			files.erase(files.begin() + i);
		}
	}

	for (size_t i = 0; i < files.size(); i++)
	{
		const auto& fileData = Utility::ReadFileToVector(files[i]);
		Utility::MemoryReader reader((char*)fileData.data(), fileData.size());
		TSTexture tex{};
		tex.LoadTexture(reader);
		std::string outPath = outputDirectory;
		if (!outPath.ends_with('\\'))
			outPath.append("\\");
		outPath += files[i];

		std::filesystem::create_directories(Utility::GetPathName(outPath));
		outPath = Utility::ReplaceExtensionWith(outPath, ".png");
		if (!tex.ExportToPNG(outPath))
		{
			Utility::Print("Failed to export %s to %s", files[i],outPath);
			Utility::Break();
			return false;
		}
	}
	return true;
}


enum ModelType
{
	Unmapped,
	HumanCharacter,
	Monkey,
	TripodTurret,
};

std::unordered_map<std::string, ModelType> ModelToModelTypeMap =
{
	std::pair<std::string, ModelType>
	{"", ModelType::Unmapped},
	{"chrincs\\chr01", ModelType::HumanCharacter},	//Colonel (Wild West)
	{"chrincs\\chr02", ModelType::HumanCharacter},	//Jungle Queen (Aztec)
	{"chrincs\\chr03", ModelType::Monkey},			//Monkey (Aztec)
	{"chrincs\\chr04", ModelType::HumanCharacter},	//Large Stone Golem (Aztec)
	{"chrincs\\chr05", ModelType::HumanCharacter},	//Large Wood Golem (Aztec)
	{"chrincs\\chr06", ModelType::HumanCharacter},	// ??? (White Woman, Siberia-esque)
	{"chrincs\\chr07", ModelType::HumanCharacter},	// ??? (Black man, Chicago-esque) 
	{"chrincs\\chr08", ModelType::HumanCharacter},	//Hector Baboso (Wild West)
	{"chrincs\\chr09", ModelType::HumanCharacter},	//Jared Slim (Wild West)

	{"chrincs\\chr10", ModelType::HumanCharacter},	//Jebediah Crump (Wild West)
	{"chrincs\\chr11", ModelType::HumanCharacter},	//Ample Sally (Wild West)
	{"chrincs\\chr12", ModelType::HumanCharacter},	//Lean Molly (Wild West)
	{"chrincs\\chr13", ModelType::HumanCharacter},	//Braces (Chicago)
	{"chrincs\\chr14", ModelType::HumanCharacter},	//Big Tony (Chicago)
	{"chrincs\\chr15", ModelType::HumanCharacter},	//Viola (Notre dam)
	{"chrincs\\chr16", ModelType::HumanCharacter},	//Stone Golem (Aztec)
	{"chrincs\\chr17", ModelType::TripodTurret},	//Motion Robot Turret (Machine Wars) (Tripod)
	//{"chrincs\\chr18", ModelType::HumanCharacter},//Doesn't Exist
	{"chrincs\\chr19", ModelType::HumanCharacter},	//Nikolai (Siberia)

	{"chrincs\\chr20", ModelType::HumanCharacter},	//Nikolai Yellow Reskin (Siberia)
	{"chrincs\\chr21", ModelType::HumanCharacter},	//Nikolai Green Reskin (Siberia)
	{"chrincs\\chr22", ModelType::HumanCharacter},	//Nikolai Blue Reskin (Siberia)
	{"chrincs\\chr23", ModelType::HumanCharacter},	//Private Poorly (Siberia) Undead
	{"chrincs\\chr24", ModelType::HumanCharacter},	//Sgt Rock (Siberia)
	{"chrincs\\chr25", ModelType::HumanCharacter},	//Sgt Shivers (Siberia)
	{"chrincs\\chr26", ModelType::HumanCharacter},	//Sgt Wood (Siberia)
	{"chrincs\\chr27", ModelType::HumanCharacter},	//Sgt Shock (Siberia)
	{"chrincs\\chr28", ModelType::HumanCharacter},	//Sgt Slate (Siberia) Undead
	{"chrincs\\chr29", ModelType::HumanCharacter},	//Lt Frost (Siberia)

	{"chrincs\\chr30", ModelType::HumanCharacter},	//Lt Wild (Siberia)
	{"chrincs\\chr31", ModelType::HumanCharacter},	//Handyman (Arcade)
	{"chrincs\\chr32", ModelType::HumanCharacter},	//Lt Shade (Siberia)
	{"chrincs\\chr33", ModelType::HumanCharacter},	//Lt Bush (Siberia)
	{"chrincs\\chr34", ModelType::HumanCharacter},	//Lt Chill (Siberia) Undead
	{"chrincs\\chr35", ModelType::HumanCharacter},	//Wood Golem (Aztec)
	{"chrincs\\chr36", ModelType::HumanCharacter},	//Trooper White (Siberia)
	{"chrincs\\chr37", ModelType::HumanCharacter},	//Trooper Brown (Siberia)
	{"chrincs\\chr38", ModelType::HumanCharacter},	//Trooper Black (Siberia)
	{"chrincs\\chr39", ModelType::HumanCharacter},	//Trooper Green (Siberia)

	{"chrincs\\chr40", ModelType::HumanCharacter},	//Trooper Grey (Siberia) Undead
	{"chrincs\\chr41", ModelType::HumanCharacter},	//Cpt Snow (Siberia)
	{"chrincs\\chr42", ModelType::HumanCharacter},	//Cpt Sand (Siberia)
	{"chrincs\\chr43", ModelType::HumanCharacter},	//Cpt Night (Siberia)
	{"chrincs\\chr44", ModelType::HumanCharacter},	//Cpt Forest (Siberia)
	{"chrincs\\chr45", ModelType::HumanCharacter},	//Cpt Pain (Siberia) Undead
	{"chrincs\\chr46", ModelType::HumanCharacter},	//Mutant (Siberia)
	{"chrincs\\chr47", ModelType::HumanCharacter},	//Ilsa Nadir (Siberia)
	{"chrincs\\chr48", ModelType::HumanCharacter},	//SentryBot (Machine Wars)
	{"chrincs\\chr49", ModelType::HumanCharacter},	//Dark Machinist Child (Machine Wars)

	{"chrincs\\chr50", ModelType::HumanCharacter},	//ChassisBot (Machine Wars)
	{"chrincs\\chr51", ModelType::HumanCharacter},	//Gretel MKII (Machine Wars)
	{"chrincs\\chr52", ModelType::HumanCharacter},	//Undead Priest (Notre Dam)
	{"chrincs\\chr53", ModelType::HumanCharacter},	//Louie Bignose (Chicago)
	{"chrincs\\chr54", ModelType::HumanCharacter},	//Marco The Snitch (Chicago)
	{"chrincs\\chr55", ModelType::HumanCharacter},	//Sewer Zombie (Notre Dam) Undead
	{"chrincs\\chr56", ModelType::HumanCharacter},	//Reaper Splitter (Space Station)
	{"chrincs\\chr57", ModelType::HumanCharacter},	//Maiden (Notre Dam)
	{"chrincs\\chr58", ModelType::HumanCharacter},	//Changeling (Notre Dam)
	{"chrincs\\chr59", ModelType::HumanCharacter},	//Portal Daemon (Notre Dam) (HUGE)

	{"chrincs\\chr60", ModelType::HumanCharacter},	//The Master (Aztec)
	{"chrincs\\chr61", ModelType::HumanCharacter},	//Krayola (NeoTokyo)
	{"chrincs\\chr62", ModelType::HumanCharacter},	//Chastity (NeoTokyo)
	{"chrincs\\chr63", ModelType::HumanCharacter},	//Jo-Beth Casey (Arcade) (Not a SLUT)
	{"chrincs\\chr64", ModelType::HumanCharacter},	//Skeleton (unused)
	{"chrincs\\chr65", ModelType::HumanCharacter},	//Leo Crupps (Arcade)
	{"chrincs\\chr66", ModelType::HumanCharacter},	//Slick Tommy (Chicago)
	{"chrincs\\chr67", ModelType::HumanCharacter},	//Jimmy Needles (Chicago)
	{"chrincs\\chr68", ModelType::HumanCharacter},	//Hatchet Sal (Chicago)
	{"chrincs\\chr69", ModelType::HumanCharacter},	//Jake Fenton

	{"chrincs\\chr70", ModelType::HumanCharacter},	//Lady Jane (Chicago)
	{"chrincs\\chr71", ModelType::HumanCharacter},	//Male Civilian (NeoTokyo)
	{"chrincs\\chr72", ModelType::HumanCharacter},	//Male Civilian (NeoTokyo)
	{"chrincs\\chr73", ModelType::HumanCharacter},	//Male Civilian (NeoTokyo)
	{"chrincs\\chr74", ModelType::HumanCharacter},	//Male Civilian (NeoTokyo)
	{"chrincs\\chr75", ModelType::HumanCharacter},	//Male Civilian (NeoTokyo)
	{"chrincs\\chr76", ModelType::HumanCharacter},	//Male Civilian (NeoTokyo)
	{"chrincs\\chr77", ModelType::HumanCharacter},	//Male Civilian (NeoTokyo)
	{"chrincs\\chr78", ModelType::HumanCharacter},	//Male Civilian (NeoTokyo)
	{"chrincs\\chr79", ModelType::HumanCharacter},	//Male Civilian (NeoTokyo)

	{"chrincs\\chr80", ModelType::HumanCharacter},	//Male Civilian (NeoTokyo)
	{"chrincs\\chr81", ModelType::HumanCharacter},	//Female Civilian (NeoTokyo)
	{"chrincs\\chr82", ModelType::HumanCharacter},	//Female Civilian (NeoTokyo)
	{"chrincs\\chr83", ModelType::HumanCharacter},	//Female Civilian (NeoTokyo)
	{"chrincs\\chr84", ModelType::HumanCharacter},	//Female Civilian (NeoTokyo)
	{"chrincs\\chr85", ModelType::HumanCharacter},	//Female Civilian (NeoTokyo)
	{"chrincs\\chr86", ModelType::HumanCharacter},	//Female Civilian (NeoTokyo)
	{"chrincs\\chr87", ModelType::HumanCharacter},	//Female Civilian (NeoTokyo)
	{"chrincs\\chr88", ModelType::HumanCharacter},	//Female Civilian (NeoTokyo)
	{"chrincs\\chr89", ModelType::HumanCharacter},	//Female Civilian (NeoTokyo)

	{"chrincs\\chr90", ModelType::HumanCharacter},	//Female Civilian (NeoTokyo)
	{"chrincs\\chr91", ModelType::HumanCharacter},	//Salaryman (NeoTokyo)
	{"chrincs\\chr92", ModelType::HumanCharacter},	//Salaryman (NeoTokyo)
	{"chrincs\\chr93", ModelType::HumanCharacter},	//Salaryman (NeoTokyo)
	{"chrincs\\chr94", ModelType::HumanCharacter},	//Feeder Zombie (Siberia) Undead
	{"chrincs\\chr95", ModelType::HumanCharacter},	//Trooper ?? Gas Mask (Siberia) 
	{"chrincs\\chr96", ModelType::HumanCharacter},	//Cyberfairy (Arcade)
	{"chrincs\\chr97", ModelType::HumanCharacter},	//R-107 (Machine Wars) (Robot?)
	{"chrincs\\chr98", ModelType::HumanCharacter},	//Captain Ash (Aztec)
	{"chrincs\\chr99", ModelType::HumanCharacter},	//Milkbaby (NeoTokyo)

	{"chrincs\\chr100", ModelType::HumanCharacter},	//Sadako (NeoTokyo)
	{"chrincs\\chr101", ModelType::HumanCharacter},	//Ghost (NeoTokyo) 
	{"chrincs\\chr102", ModelType::HumanCharacter},	//Barby Gimp (NeoTokyo)
	{"chrincs\\chr103", ModelType::HumanCharacter},	//Riot Officer (NeoTokyo)
	{"chrincs\\chr104", ModelType::HumanCharacter},	//Patient (Unused)
	{"chrincs\\chr105", ModelType::HumanCharacter},	//Jacque-de-la-Morte (Notre Dam)
	{"chrincs\\chr106", ModelType::HumanCharacter},	//Hunchback (Notre Dam)
	{"chrincs\\chr107", ModelType::HumanCharacter},	//Cortez (Space Station) It's Time to Split!
	{"chrincs\\chr108", ModelType::HumanCharacter},	//Hank Nova (Planet-X)
	{"chrincs\\chr109", ModelType::HumanCharacter},	//Kitten Celeste (Atom Smasher)

	{"chrincs\\chr110", ModelType::HumanCharacter},	//Bear (Arcade)
	{"chrincs\\chr111", ModelType::HumanCharacter},	//Stumpy (Arcade)
	{"chrincs\\chr112", ModelType::HumanCharacter},	//Gregor Lenko (Siberia)
	{"chrincs\\chr113", ModelType::HumanCharacter},	//Mikey Two-guns (Wild West) 
	{"chrincs\\chr114", ModelType::HumanCharacter},	//Venus Starr (Wild West)
	{"chrincs\\chr115", ModelType::HumanCharacter},	//Harry Tipper (Atom Smasher) All right dude!
	{"chrincs\\chr116", ModelType::HumanCharacter},	//Henchman (Atom Smasher)
	{"chrincs\\chr117", ModelType::HumanCharacter},	//Dr Peabody (Atom Smasher)
	{"chrincs\\chr118", ModelType::HumanCharacter},	//Khallos (Atom Smasher)
	{"chrincs\\chr119", ModelType::HumanCharacter},	//Aztec Warrior (Aztec)

	{"chrincs\\chr120", ModelType::HumanCharacter},	//High Priest (Aztec)
	{"chrincs\\chr121", ModelType::HumanCharacter},	//Mr Giggles (Arcade) Wanna see my pocket?
	{"chrincs\\chr122", ModelType::HumanCharacter},	//Kypriss (Arcade)
	{"chrincs\\chr123", ModelType::HumanCharacter},	//Dinosaur (Aztec)
	{"chrincs\\chr124", ModelType::HumanCharacter},	//Ozor Mox (Planet-X)
	{"chrincs\\chr125", ModelType::HumanCharacter},	//Meezor Mox (Planet-X)
	{"chrincs\\chr126", ModelType::HumanCharacter},	//Candi Skyler (Planet-X)
	{"chrincs\\chr127", ModelType::HumanCharacter},	//Scourge Splitter (Space Station)
	{"chrincs\\chr128", ModelType::HumanCharacter},	//Corporal Heart (Space Station)
	{"chrincs\\chr129", ModelType::HumanCharacter},	//Drone Splitter (Space Station)

	{"chrincs\\chr130", ModelType::HumanCharacter},	//Cropolite (Notre Dam)
	{"chrincs\\chr131", ModelType::HumanCharacter},	//Female Trooper (Space Station)
	{"chrincs\\chr132", ModelType::HumanCharacter},	//Male Trooper (Space Station)
	{"chrincs\\chr133", ModelType::HumanCharacter},	//R109 (Machine Wars)
	{"chrincs\\chr134", ModelType::HumanCharacter},	//Mr Underwood (Notre Dam)
	{"chrincs\\chr135", ModelType::HumanCharacter},	//Gargoyle (Notre Dam)
	{"chrincs\\chr136", ModelType::HumanCharacter},	//Crypt Zombie (Notre Dam) Undead
	{"chrincs\\chr137", ModelType::HumanCharacter},	//Lola Varuska (Arcade)
	{"chrincs\\chr138", ModelType::HumanCharacter},	//Nikki (Arcade)
	{"chrincs\\chr139", ModelType::HumanCharacter},	//Jinki (Arcade)

	{"chrincs\\chr140", ModelType::HumanCharacter},	//Ringmistress (Arcade)
	{"chrincs\\chr141", ModelType::HumanCharacter},	//Snowman (Arcade) I'm melting!
	{"chrincs\\chr142", ModelType::HumanCharacter},	//Crispin (Arcade) Burn Baby Burn!
	{"chrincs\\chr143", ModelType::HumanCharacter},	//Baby Drone Splitter (Space Station)
	{"chrincs\\chr144", ModelType::HumanCharacter},	//Calamari (Arcade)
	{"chrincs\\chr145", ModelType::HumanCharacter},	//Dark Henchman (Atom Smasher)
	{"chrincs\\chr146", ModelType::HumanCharacter},	//Sentrybot (Machine Wars)
	//{"chrincs\\chr147", ModelType::HumanCharacter},//Doesn't Exist
	//{"chrincs\\chr148", ModelType::HumanCharacter},//Doesn't Exist
	{"chrincs\\chr149", ModelType::HumanCharacter},	//Sergio (Arcade)

	{"chrincs\\chr150", ModelType::HumanCharacter},	//Beetleman (Arcade)
	{"chrincs\\chr151", ModelType::HumanCharacter},	//Mischief (Arcade)
	{"chrincs\\chr152", ModelType::HumanCharacter},	//Impersonator (Arcade)
	{"chrincs\\chr153", ModelType::HumanCharacter},	//Bad-ass Cyborg (Arcade)
	{"chrincs\\chr154", ModelType::HumanCharacter},	//Chinese Chef (Arcade)
	{"chrincs\\chr155", ModelType::HumanCharacter},	//Duck (Arcade)
	{"chrincs\\chr156", ModelType::HumanCharacter},	//Gingerbread Man (Arcade)
	{"chrincs\\chr157", ModelType::HumanCharacter}, //Insect Mutant (Arcade)
	{"chrincs\\chr158", ModelType::HumanCharacter},	//Robofish (Arcade)

};

struct AnimationPair
{
	std::string path;
	TSAnimation animation;
};
std::unordered_map<ModelType, std::vector<AnimationPair>> ModelTypeToAnimationMap =
{
	std::pair<ModelType,std::vector<AnimationPair>>
	{Unmapped, {}},
	{HumanCharacter, 
		std::vector<AnimationPair>
		{
			AnimationPair
			{"\\data\\ts2\\1hndhigh_m0.raw",{}},
			{"\\data\\ts2\\2gnlow_m0.raw",{}},
			{"\\data\\ts2\\2hndlow_m0.raw",{}},
			{"\\data\\ts2\\2pistls.raw",{}},
			{"\\data\\ts2\\alertpatrol_m0.raw",{}},
			{"\\data\\ts2\\backflip5_m0.raw",{}},
			{"\\data\\ts2\\barrelbindpose2_m0.raw",{}},
			{"\\data\\ts2\\barreldeath_m0.raw",{}},
			{"\\data\\ts2\\barreldroptoturn_m0.raw",{}},
			{"\\data\\ts2\\barrelendturn_m0.raw",{}},
			{"\\data\\ts2\\barrelstandtowalk_m0.raw",{}},
			{"\\data\\ts2\\barrelstartturn_m0.raw",{}},
			{"\\data\\ts2\\barreltoturret_m0.raw",{}},
			{"\\data\\ts2\\barrelturntostand_m0.raw",{}},
			{"\\data\\ts2\\barrelturrettostand_m0.raw",{}},
			{"\\data\\ts2\\barrelubix_m0.raw",{}},
			{"\\data\\ts2\\barrelwalk_m0.raw",{}},
			{"\\data\\ts2\\bar_breath_m0.raw",{}},
			{"\\data\\ts2\\bar_getup_m0.raw",{}},
			{"\\data\\ts2\\bar_sit_hickup_m0.raw",{}},
			{"\\data\\ts2\\BigBot_Death_m0.raw",{}},
			{"\\data\\ts2\\BigBot_walk_m0.raw",{}},
			{"\\data\\ts2\\bm_death_m0.raw",{}},
			{"\\data\\ts2\\bm_walk_m0.raw",{}},
			{"\\data\\ts2\\cartwheel_m0.raw",{}},
			{"\\data\\ts2\\chestshot1_m0.raw",{}},
			{"\\data\\ts2\\chestshot2_m0.raw",{}},
			{"\\data\\ts2\\cowgirl_breathe.raw",{}},
			{"\\data\\ts2\\cowgirl_idol.raw",{}},
			{"\\data\\ts2\\crchrun_back_m0.raw",{}},
			{"\\data\\ts2\\crchrun_forw_m0.raw",{}},
			{"\\data\\ts2\\crchrun_left_m0.raw",{}},
			{"\\data\\ts2\\crchstill_m0.raw",{}},
			{"\\data\\ts2\\crchwlk_rgt_m0.raw",{}},
			{"\\data\\ts2\\crouch2_m0.raw",{}},
			{"\\data\\ts2\\crouchnew1_m0.raw",{}},
			{"\\data\\ts2\\csp_m0.raw",{}},
			{"\\data\\ts2\\cwr_m00.raw",{}},
			{"\\data\\ts2\\c_2_s_m0.raw",{}},
			{"\\data\\ts2\\c_gran_m0.raw",{}},
			{"\\data\\ts2\\c_s_2_c_m0.raw",{}},
			{"\\data\\ts2\\c_s_m0.raw",{}},
			{"\\data\\ts2\\c_s_noi_m0.raw",{}},
			{"\\data\\ts2\\diagbackright2_m0.raw",{}},
			{"\\data\\ts2\\diagforleft_m0.raw",{}},
			{"\\data\\ts2\\dive_m0.raw",{}},
			{"\\data\\ts2\\dodgeleftright_m0.raw",{}},
			{"\\data\\ts2\\doubledodge4_m0.raw",{}},
			{"\\data\\ts2\\drillhed_texboned_m0.raw",{}},
			{"\\data\\ts2\\droptoknee_m0.raw",{}},
			{"\\data\\ts2\\electroend_m0.raw",{}},
			{"\\data\\ts2\\electroloop_m0.raw",{}},
			{"\\data\\ts2\\expld2_m0.raw",{}},
			{"\\data\\ts2\\expld3_m0.raw",{}},
			{"\\data\\ts2\\faintleft3_m0.raw",{}},
			{"\\data\\ts2\\fall_m0.raw",{}},
			{"\\data\\ts2\\Fire_hide2_m0.raw",{}},
			{"\\data\\ts2\\Fire_hide_m0.raw",{}},
			{"\\data\\ts2\\fire_pose2_m0.raw",{}},
			{"\\data\\ts2\\fire_pose_m0.raw",{}},
			{"\\data\\ts2\\flipped_doubledodge_m0.raw",{}},
			{"\\data\\ts2\\flipped_faintright_m0.raw",{}},
			{"\\data\\ts2\\flipped_rollleft_m0.raw",{}},
			{"\\data\\ts2\\fronthit_m0.raw",{}},
			{"\\data\\ts2\\getup1_m0.raw",{}},
			{"\\data\\ts2\\getup2_m0.raw",{}},
			{"\\data\\ts2\\golem_punchgrd_m0.raw",{}},
			{"\\data\\ts2\\golem_run_m0.raw",{}},
			{"\\data\\ts2\\golem_swipeleft_m0.raw",{}},
			{"\\data\\ts2\\golem_swiperight_m0.raw",{}},
			{"\\data\\ts2\\grenade2_m0.raw",{}},
			{"\\data\\ts2\\grenade_m0.raw",{}},
			{"\\data\\ts2\\heartshot_m0.raw",{}},
			{"\\data\\ts2\\Hide_fire2_m0.raw",{}},
			{"\\data\\ts2\\Hide_fire_m0.raw",{}},
			{"\\data\\ts2\\Hide_pose2_m0.raw",{}},
			{"\\data\\ts2\\Hide_pose_m0.raw",{}},
			{"\\data\\ts2\\highatck1_m0.raw",{}},
			{"\\data\\ts2\\highatck2_m0.raw",{}},
			{"\\data\\ts2\\hit_1_m0.raw",{}},
			{"\\data\\ts2\\hit_2_m0.raw",{}},
			{"\\data\\ts2\\hit_3_m0.raw",{}},
			{"\\data\\ts2\\hit_4_m0.raw",{}},
			{"\\data\\ts2\\hit_5_m0.raw",{}},
			{"\\data\\ts2\\Hit_react1_m0.raw",{}},
			{"\\data\\ts2\\Hit_react2_m0.raw",{}},
			{"\\data\\ts2\\hldmine.raw",{}},
			{"\\data\\ts2\\human_19_bindpose.raw",{}},
			{"\\data\\ts2\\Hydrent_spray_m0.raw",{}},
			{"\\data\\ts2\\idl_arse_m0.raw",{}},
			{"\\data\\ts2\\idl_wpbrw_m0.raw",{}},
			{"\\data\\ts2\\lbarm_sp_m03.raw",{}},
			{"\\data\\ts2\\lbarm_ubx_m02.raw",{}},
			{"\\data\\ts2\\leaning_bar_look_m0.raw",{}},
			{"\\data\\ts2\\leaning_bar_m0.raw",{}},
			{"\\data\\ts2\\leftshoulder_m0.raw",{}},
			{"\\data\\ts2\\leftstomach_m0.raw",{}},
			{"\\data\\ts2\\lookatgun_m0.raw",{}},
			{"\\data\\ts2\\look_m0.raw",{}},
			{"\\data\\ts2\\lowandleft_m0.raw",{}},
			{"\\data\\ts2\\lsarm_sp_m01.raw",{}},
			{"\\data\\ts2\\lsarm_ubx_m01.raw",{}},
			{"\\data\\ts2\\monkey_beat.raw",{}},
			{"\\data\\ts2\\monkey_flip.raw",{}},
			{"\\data\\ts2\\monkey_flying.raw",{}},
			{"\\data\\ts2\\monkey_flytumble.raw",{}},
			{"\\data\\ts2\\monkey_horn.raw",{}},
			{"\\data\\ts2\\monkey_melon.raw",{}},
			{"\\data\\ts2\\monkey_pipe.raw",{}},
			{"\\data\\ts2\\monkey_scratch.raw",{}},
			{"\\data\\ts2\\monkey_stand.raw",{}},
			{"\\data\\ts2\\monkey_tumble.raw",{}},
			{"\\data\\ts2\\moveforward_m0.raw",{}},
			{"\\data\\ts2\\moveleft_m0.raw",{}},
			{"\\data\\ts2\\moveright_m0.raw",{}},
			{"\\data\\ts2\\nailhed_texboned_m0.raw",{}},
			{"\\data\\ts2\\onfire_m0.raw",{}},
			{"\\data\\ts2\\onknee_m0.raw",{}},
			{"\\data\\ts2\\patrol2_m0.raw",{}},
			{"\\data\\ts2\\patrol_m0.raw",{}},
			{"\\data\\ts2\\peek2_m0.raw",{}},
			{"\\data\\ts2\\peek_m0.raw",{}},
			{"\\data\\ts2\\pickingupphone_m0.raw",{}},
			{"\\data\\ts2\\pistolpointing_m0.raw",{}},
			{"\\data\\ts2\\pnch_stk_m0.raw",{}},
			{"\\data\\ts2\\pointalngls2handed_m0.raw",{}},
			{"\\data\\ts2\\priestcross_m0.raw",{}},
			{"\\data\\ts2\\prieststand_m0.raw",{}},
			{"\\data\\ts2\\punchflip_m0.raw",{}},
			{"\\data\\ts2\\punch_m0.raw",{}},
			{"\\data\\ts2\\punch_mpc_m0.raw",{}},
			{"\\data\\ts2\\pushbutton_m0.raw",{}},
			{"\\data\\ts2\\railspiderbindpose.raw",{}},
			{"\\data\\ts2\\rbarm_sp_m02.raw",{}},
			{"\\data\\ts2\\rbarm_ubx_m03.raw",{}},
			{"\\data\\ts2\\reaction_3_m0.raw",{}},
			{"\\data\\ts2\\reloadpistol_m0.raw",{}},
			{"\\data\\ts2\\reloadrifle_m0.raw",{}},
			{"\\data\\ts2\\restani1_m0.raw",{}},
			{"\\data\\ts2\\restani2_m0.raw",{}},
			{"\\data\\ts2\\retreat_m0.raw",{}},
			{"\\data\\ts2\\rightleg_m0.raw",{}},
			{"\\data\\ts2\\rightshoulder_m0.raw",{}},
			{"\\data\\ts2\\rightstomach_m0.raw",{}},
			{"\\data\\ts2\\robot_activate_m0.raw",{}},
			{"\\data\\ts2\\rollright_m0.raw",{}},
			{"\\data\\ts2\\rsarm_sp_m04.raw",{}},
			{"\\data\\ts2\\rsarm_ubx_m04.raw",{}},
			{"\\data\\ts2\\runback_m0.raw",{}},
			{"\\data\\ts2\\runleft_m0.raw",{}},
			{"\\data\\ts2\\runright_m0.raw",{}},
			{"\\data\\ts2\\run_m0.raw",{}},
			{"\\data\\ts2\\sawarm_texboned_m0.raw",{}},
			{"\\data\\ts2\\scientist_defusebomb_m0.raw",{}},
			{"\\data\\ts2\\scientist_run_m0.raw",{}},
			{"\\data\\ts2\\scientist_stand_m0.raw",{}},
			{"\\data\\ts2\\scratchhead.raw",{}},
			{"\\data\\ts2\\searchpatrol2_gun_m0.raw",{}},
			{"\\data\\ts2\\searchpatrol2_m0.raw",{}},
			{"\\data\\ts2\\sholdershot2_m0.raw",{}},
			{"\\data\\ts2\\Shotgun_hit_m0.raw",{}},
			{"\\data\\ts2\\shothead2_m0.raw",{}},
			{"\\data\\ts2\\shothead3_m0.raw",{}},
			{"\\data\\ts2\\shothead_m0.raw",{}},
			{"\\data\\ts2\\shotshldrturn_m0.raw",{}},
			{"\\data\\ts2\\shouldershot3_m0.raw",{}},
			{"\\data\\ts2\\shouldershot_m0.raw",{}},
			{"\\data\\ts2\\shuffle_left_m0.raw",{}},
			{"\\data\\ts2\\shuffle_m0.raw",{}},
			{"\\data\\ts2\\sidestep_rightlittle_m0.raw",{}},
			{"\\data\\ts2\\sidestep_right_m0.raw",{}},
			{"\\data\\ts2\\slide_m0.raw",{}},
			{"\\data\\ts2\\smallstep10_m0.raw",{}},
			{"\\data\\ts2\\smallstep9_m0.raw",{}},
			{"\\data\\ts2\\smallstepback_m0.raw",{}},
			{"\\data\\ts2\\spit_m0.raw",{}},
			{"\\data\\ts2\\spose_m0.raw",{}},
			{"\\data\\ts2\\stand1_m0.raw",{}},
			{"\\data\\ts2\\stand2_m0.raw",{}},
			{"\\data\\ts2\\standfaintleft_m0.raw",{}},
			{"\\data\\ts2\\standfaintright_m0.raw",{}},
			{"\\data\\ts2\\standfromknee_m0.raw",{}},
			{"\\data\\ts2\\standpose_m0.raw",{}},
			{"\\data\\ts2\\standtorunback_m0.raw",{}},
			{"\\data\\ts2\\standtorunleft_m0.raw",{}},
			{"\\data\\ts2\\standtorunright_m0.raw",{}},
			{"\\data\\ts2\\standtorun_m0.raw",{}},
			{"\\data\\ts2\\standtowalk180_m0.raw",{}},
			{"\\data\\ts2\\standtowalkleft_m0.raw",{}},
			{"\\data\\ts2\\standtowalkright_m0.raw",{}},
			{"\\data\\ts2\\standtowalk_m0.raw",{}},
			{"\\data\\ts2\\stepback5_m0.raw",{}},
			{"\\data\\ts2\\stepright_m0.raw",{}},
			{"\\data\\ts2\\stnd2_m0.raw",{}},
			{"\\data\\ts2\\stnd3_m0.raw",{}},
			{"\\data\\ts2\\stnd4_m0.raw",{}},
			{"\\data\\ts2\\stndshfl.raw",{}},
			{"\\data\\ts2\\stndwtch.raw",{}},
			{"\\data\\ts2\\stnd_m0.raw",{}},
			{"\\data\\ts2\\stomachhit_m0.raw",{}},
			{"\\data\\ts2\\straifforleft_m0.raw",{}},
			{"\\data\\ts2\\straifforright_m0.raw",{}},
			{"\\data\\ts2\\strt_atck_m0.raw",{}},
			{"\\data\\ts2\\summersault2_m0.raw",{}},
			{"\\data\\ts2\\swipe_1_m0.raw",{}},
			{"\\data\\ts2\\swipe_2_m0.raw",{}},
			{"\\data\\ts2\\swipe_3_m0.raw",{}},
			{"\\data\\ts2\\swipe_4_m0.raw",{}},
			{"\\data\\ts2\\talkingphone_m0.raw",{}},
			{"\\data\\ts2\\trnsf_blokedeath_m0.raw",{}},
			{"\\data\\ts2\\trnsf_blokesleep_m0.raw",{}},
			{"\\data\\ts2\\trnsf_blokeup_m0.raw",{}},
			{"\\data\\ts2\\trnsf_tsbreathe_m0.raw",{}},
			{"\\data\\ts2\\trnsf_tsdeath_m0.raw",{}},
			{"\\data\\ts2\\trnsf_tssleep_m0.raw",{}},
			{"\\data\\ts2\\trnsf_tsthrowleft_m0.raw",{}},
			{"\\data\\ts2\\trnsf_tsthrowright_m0.raw",{}},
			{"\\data\\ts2\\trnsf_tswake_m0.raw",{}},
			{"\\data\\ts2\\ts_run_m0.raw",{}},
			{"\\data\\ts2\\ts_spin_m0.raw",{}},
			{"\\data\\ts2\\ts_stand_m0.raw",{}},
			{"\\data\\ts2\\turn90_l1_m0.raw",{}},
			{"\\data\\ts2\\turn90_l2_m0.raw",{}},
			{"\\data\\ts2\\turn90_r1_m0.raw",{}},
			{"\\data\\ts2\\turn90_r2_m0.raw",{}},
			{"\\data\\ts2\\turn90_r3_m0.raw",{}},
			{"\\data\\ts2\\turn90_stretchright_m0.raw",{}},
			{"\\data\\ts2\\turnnrun_m0.raw",{}},
			{"\\data\\ts2\\typing_cycle_m00.raw",{}},
			{"\\data\\ts2\\vrgn_chaindeath_m0.raw",{}},
			{"\\data\\ts2\\vrgn_chn_rls_m0.raw",{}},
			{"\\data\\ts2\\vrgn_chn_slp_m0.raw",{}},
			{"\\data\\ts2\\vrgn_chn_slp_zom_m0.raw",{}},
			{"\\data\\ts2\\vrgn_chn_wrgl_m0.raw",{}},
			{"\\data\\ts2\\vrgn_hangdeath_m0.raw",{}},
			{"\\data\\ts2\\vrgn_hang_m0.raw",{}},
			{"\\data\\ts2\\vrgn_hang_rls_m0.raw",{}},
			{"\\data\\ts2\\vrgn_scared_m0.raw",{}},
			{"\\data\\ts2\\vrgn_walk_m0.raw",{}},
			{"\\data\\ts2\\walk2_m0.raw",{}},
			{"\\data\\ts2\\walk3_m0.raw",{}},
			{"\\data\\ts2\\walk4_m0.raw",{}},
			{"\\data\\ts2\\walkback_m0.raw",{}},
			{"\\data\\ts2\\walkleft_m0.raw",{}},
			{"\\data\\ts2\\walkright_m0.raw",{}},
			{"\\data\\ts2\\walktostand_m0.raw",{}},
			{"\\data\\ts2\\walk_m0.raw",{}},
			{"\\data\\ts2\\wlk1_m0.raw",{}},
			{"\\data\\ts2\\wlk4_m0.raw",{}},
			{"\\data\\ts2\\wlk5_m0.raw",{}},
			{"\\data\\ts2\\zombiestand_m0.raw", {}},
		},
	},
};


bool TS2Importer::ModelConvert(const std::string& pathToExtractedData, const std::string& outputDirectory)
{

	std::string modelPath = Utility::ReplaceChar(pathToExtractedData, '/', '\\');;
	if (!modelPath.ends_with('\\'))
		modelPath.append("\\");

	std::string animationPath = modelPath;

	animationPath.append("anim");
	modelPath.append("ob");
	std::vector<std::string> modelFiles = Utility::GetFilesInDirectoryRecursive(modelPath);

	for (size_t i = modelFiles.size() - 1; i < modelFiles.size(); i--)
	{
		if (!modelFiles[i].ends_with(".raw"))
		{
			modelFiles.erase(modelFiles.begin() + i);
		}
	}
	

	for (auto& pair : ModelTypeToAnimationMap)
	{
		for (auto& animPair : pair.second)
		{
			std::vector<uint8_t> fileData = Utility::ReadFileToVector(animationPath + animPair.path);
			Utility::MemoryReader reader((char*)fileData.data(), fileData.size());
			animPair.animation.Load(reader);
			animPair.animation.name = Utility::GetFileName(animPair.path);
		}
	}

	for (size_t i = 0; i < modelFiles.size(); i++)
	{
		const auto& fileData = Utility::ReadFileToVector(modelFiles[i]);
		Utility::MemoryReader reader((char*)fileData.data(), fileData.size());
		TSModel model{};
		model.Load(reader);
		std::string outPath = outputDirectory;
		if (!outPath.ends_with('\\'))
			outPath.append("\\");
		outPath += modelFiles[i];

		if (Utility::Contains(modelFiles[i], "\\chrincs\\", false))
		{
			std::string filename = "chrincs\\" + Utility::ReplaceExtensionWith(Utility::GetFileName(modelFiles[i]), "");
			const auto& it = ModelToModelTypeMap.find(filename);
			if (it != ModelToModelTypeMap.end())
			{
				const auto& pair = ModelTypeToAnimationMap[it->second];
				static TSSkeleton humanSkeleton = TSSkeleton::GetHumanSkeleton();
				model.AddSkeleton(humanSkeleton);
				for (const auto& anim : pair)
				{
					model.AddAnimation(anim.animation);
				}
			}
			else
			{
				Utility::Print("Unable to find: %s in modeltypemap", filename.c_str());
				Utility::Break();
			}

		}

		std::filesystem::create_directories(Utility::GetPathName(outPath));
		outPath = Utility::ReplaceExtensionWith(outPath, ".gltf");
		if (!model.ExportToGLTF(outPath))
		{
			Utility::Print("Failed to export %s to %s", modelFiles[i], outPath);
			Utility::Break();
			return false;
		}
	}
	return true;
}

void TS2Importer::PakFile::LoadEntriesV4(Utility::MemoryReader& reader)
{
	constexpr size_t ENTRY_SIZE = 60;
	size_t numEntries = pakHeader.dirSize / ENTRY_SIZE;
	reader.Seek(pakHeader.dirOffset, Utility::MemoryReader::SeekMode::BEGIN);
	for (size_t i = 0; i < numEntries; i++)
	{
		const PakEntry& entry = LoadDirEntryV4(reader);
		fileEntries.try_emplace(entry.name, entry);
	}
}

void TS2Importer::PakFile::LoadEntriesV5(Utility::MemoryReader& reader){}
void TS2Importer::PakFile::LoadEntriesV8(Utility::MemoryReader& reader){}


TS2Importer::PakFile::PakEntry TS2Importer::PakFile::LoadDirEntryV4(Utility::MemoryReader& reader)
{
	constexpr size_t FILENAME_LENGTH = 48;
	PakEntry entry;
	entry.name.resize(FILENAME_LENGTH);
	reader.Read<char, FILENAME_LENGTH>(entry.name.data());

	entry.offset = reader.Read<uint32_t>();
	entry.size = reader.Read<uint32_t>();
	entry.extra = 0; reader.Skip(sizeof(uint32_t));
	return entry;
}

TS2Importer::PakFile::PakEntry TS2Importer::PakFile::LoadDirEntryV5(Utility::MemoryReader& reader)
{
	PakEntry entry;
	uint32_t nameID = reader.Read<uint32_t>();
	entry.offset = reader.Read<uint32_t>();
	entry.size = reader.Read<uint32_t>();
	entry.extra = reader.Read<uint32_t>();

	entry.name = std::to_string(nameID);

	return entry;
}

TS2Importer::PakFile::PakEntry TS2Importer::PakFile::LoadDirEntryV8(Utility::MemoryReader& reader)
{
	PakEntry entry;
	entry.extra = reader.Read<uint32_t>();
	entry.size = reader.Read<uint32_t>();
	entry.offset = reader.Read<uint32_t>();
	//V8 has no name
	return entry;
}
