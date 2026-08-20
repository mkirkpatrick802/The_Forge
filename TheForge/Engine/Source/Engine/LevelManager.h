#pragma once
#include <string>
#include <vector>

#include "ByteStream.h"
#include "json.hpp"

namespace Engine
{
    const std::string LEVEL_PATH = "Assets/Levels/";
    
    class Level;
    class LevelManager
    {
        friend class GameEngine;
        
    public:
        
        static bool LoadLevel(const std::string& filepath);
        static bool LoadLevel(NetCode::InputByteStream& stream);
        static bool CreateLevel(const std::string& levelName);

        // Loads a level and runs its game mode's Start, and nothing else.
        //
        // Distinct from StartCurrentLevel, which is the "press Play" path: that one also
        // closes any open prefab, turns the editor off, reloads from disk a second time
        // and adopts the first camera it finds. None of that is wanted when the level
        // being opened is the main menu -- it is not a level anyone is playing.
        static bool OpenLevel(const std::string& filepath);

        // --- prefab editing ---
        //
        // A prefab is opened *as the world*, the way a level is: the open level is closed
        // and replaced by one containing nothing but the prefab. Everything the editor
        // does -- the hierarchy, selection, spawning children, saving -- then works on it
        // through the same path it uses for a level, and none of it has to know a prefab
        // is a special kind of thing.
        //
        // The alternative, keeping a prefab alive alongside the open level, does not
        // work: components live in global pools, so the prefab draws, collides and can
        // be picked out of the viewport while nothing in the level owns it.
        static bool OpenPrefab(const std::string& filepath);

        // Writes the prefab back and returns to the level that was open when it was
        // opened. Does nothing if a prefab is not being edited.
        static void ClosePrefab();

        static bool IsEditingPrefab();

        LevelManager(const std::string& filepath);
        void StartCurrentLevel() const;
        
    public:
        
        static Level* GetCurrentLevel() { return _currentLevel; }

        //Returns the list of file paths, and gives out the list of json data
        static std::vector<std::string> GetAllLevels(std::vector<nlohmann::json>& levelData);

    private:
        
        void CleanUp() const;
        
    private:

        // Drops any selection pointing into the level that is about to be destroyed.
        // Every level swap goes through here, because a stale selection is a pointer to
        // a freed GameObject that the details panel and the selection overlay both read
        // before anything else gets a chance to set it.
        static void ReleaseCurrentLevel();
        
    private:
        
        static Level* _currentLevel;

        // The level to return to when the prefab being edited is closed. Empty when no
        // level was open, and deliberately not overwritten when one prefab is opened
        // from another -- the way back is to the level, not through a chain of prefabs.
        static std::string _levelBeforePrefab;
        
    };
}
