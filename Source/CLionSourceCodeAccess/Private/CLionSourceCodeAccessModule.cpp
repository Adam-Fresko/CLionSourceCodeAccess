// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "Runtime/Core/Public/Features/IModularFeatures.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "CLionSourceCodeAccessModule.h"
#include "CLionSettings.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

IMPLEMENT_MODULE( FCLionSourceCodeAccessModule, CLionSourceCodeAccess);

bool FCLionSourceCodeAccessModule::SupportsDynamicReloading()
{
    return true;
}

void FCLionSourceCodeAccessModule::StartupModule()
{

    // Register our custom settings
    this->RegisterSettings();

    // Register our custom menu additions
    this->RegisterMenu();

    // Start her up
    CLionSourceCodeAccessor.Startup();

    // Bind our source control provider to the editor
    IModularFeatures::Get().RegisterModularFeature(TEXT("SourceCodeAccessor"), &CLionSourceCodeAccessor );
}


void FCLionSourceCodeAccessModule::ShutdownModule()
{
    CLionSourceCodeAccessor.Shutdown();

    if (UObjectInitialized())
    {
        this->UnregisterSettings();
    }

    this->UnregisterMenu();

    // unbind provider from editor
    IModularFeatures::Get().UnregisterModularFeature(TEXT("SourceCodeAccessor"), &CLionSourceCodeAccessor);
}



void FCLionSourceCodeAccessModule::RegisterSettings()
{
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "CLion",
                                         LOCTEXT("RuntimeSettingsName", "CLion"),
                                         LOCTEXT("RuntimeSettingsDescription", "Configure the CLion Integration"),
                                         GetMutableDefault<UCLionSettings>()
        );

        if (SettingsSection.IsValid())
        {
            SettingsSection->OnModified().BindRaw(this, &FCLionSourceCodeAccessModule::HandleSettingsSaved);
        }
    }
}

void FCLionSourceCodeAccessModule::RegisterMenu()
{
    if ( FModuleManager::Get().IsModuleLoaded( "LevelEditor" ) )
    {
        FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>( TEXT("LevelEditor") );

        MainMenuExtender = MakeShareable(new FExtender);
        MainMenuExtender->AddMenuExtension("FileProject", EExtensionHook::After, NULL, FMenuExtensionDelegate::CreateRaw(this, &FCLionSourceCodeAccessModule::AddMenuOptions));

        LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MainMenuExtender);
    }

}





void FCLionSourceCodeAccessModule::UnregisterSettings()
{
    // Ensure to unregister all of your registered settings here, hot-reload would
    // otherwise yield unexpected results.

    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Project", "CLionSettings", "General");
    }
}

void FCLionSourceCodeAccessModule::UnregisterMenu() {

}



void FCLionSourceCodeAccessModule::AddMenuOptions(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.BeginSection("CLionMenu", LOCTEXT("CLionMenuLabel", "CLion"));

    MenuBuilder.AddMenuEntry(
            LOCTEXT("CLionMenuGenerateCMakeListLabel", "Generate CMakeList"),
            LOCTEXT("CLionMenuGenerateCMakeListTooltip", "Generates the CMakeList file for the opened project."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateRaw(this, &FCLionSourceCodeAccessModule::HandleGenerateProjectFiles)));

    MenuBuilder.EndSection();
}




bool FCLionSourceCodeAccessModule::HandleSettingsSaved()
{
    UCLionSettings* Settings = GetMutableDefault<UCLionSettings>();

    if ( Settings->bRequestRefresh )
    {
        Settings->SaveConfig();
    }
    return true;
}


void FCLionSourceCodeAccessModule::HandleGenerateProjectFiles()
{
    CLionSourceCodeAccessor.GenerateProjectFile();
}

FCLionSourceCodeAccessor& FCLionSourceCodeAccessModule::GetAccessor()
{
    return CLionSourceCodeAccessor;
}
