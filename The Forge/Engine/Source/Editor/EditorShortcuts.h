#pragma once


namespace Editor
{
    class EditorShortcuts
    {
    public:
        EditorShortcuts() = default;
        ~EditorShortcuts() = default;
        
        static EditorShortcuts& GetInstance();
        void Update();
    };
}