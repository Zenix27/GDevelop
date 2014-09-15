/**

Game Develop - Tile Map Extension
Copyright (c) 2014 Victor Levasseur (victorlevasseur52@gmail.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.

*/

#if defined(GD_IDE_ONLY) && !defined(GD_NO_WX_GUI)

#include <iostream>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/dcbuffer.h>
#include <wx/textdlg.h>
#include <wx/settings.h>
#include <wx/config.h>
#include <wx/msgdlg.h>

#include "GDCore/Tools/Log.h"
#include "GDCore/Tools/Localization.h"
#include "GDCore/IDE/Dialogs/MainFrameWrapper.h"
#include "GDCore/IDE/SkinHelper.h"
#include "GDCpp/Project.h"
#include "GDCpp/CommonTools.h"
#include "GDCore/IDE/Dialogs/ResourcesEditor.h"
#include "GDCore/IDE/CommonBitmapManager.h"
#include "GDCore/Tools/HelpFileAccess.h"

#include "TileMapObjectEditor.h"
#include "TileMapObject.h"
#include "TileSetConfigurationEditor.h"
#include "TileMapConfigurationEditor.h"
#include "TileEditor.h"

TileMapObjectEditor::TileMapObjectEditor( wxWindow* parent, gd::Project & game_, TileMapObject & object_, gd::MainFrameWrapper & mainFrameWrapper_ ) :
TileMapObjectEditorBase(parent),
game(game_),
mainFrameWrapper(mainFrameWrapper_),
object(object_),
tileSet(object.tileSet.Get()),
tileMap(object.tileMap.Get()),
m_tileEditorPanel(new TileEditor(this))
{
    m_tileSetPanel->SetTileSet(&tileSet);
    m_tileEditorPanel->SetTileSet(&tileSet);
    m_tileMapPanel->SetTileSet(&tileSet);
    m_tileMapPanel->SetTileMap(&tileMap);
    m_tileSetPanel->Connect(TILE_SELECTION_CHANGED, TileSelectionEventHandler(TileMapObjectEditor::OnTileSetSelectionChanged), NULL, this);

    //Set a nice GUI theme
    m_auimgr178->GetArtProvider()->SetColour(wxAUI_DOCKART_BACKGROUND_COLOUR, wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
    gd::SkinHelper::ApplyCurrentSkin(*m_auimgr178);
    m_auimgr178->Update();

    //Add the Tile Editor
    m_auimgr178->AddPane(m_tileEditorPanel, wxAuiPaneInfo().Name(wxT("TILEEDITORPANEL"))
                                                           .DefaultPane()
                                                           .Caption(_("Tile editor"))
                                                           .CloseButton(true)
                                                           .MaximizeButton(true)
                                                           .Show(false)
                                                           .MinSize(200,200)
                        );

    //Load the perspective
    if(wxConfigBase::Get()->HasEntry("/TileMapObjectEditor/LastWorkspace"))
    {
        wxString perspective;
        wxConfigBase::Get()->Read("/TileMapObjectEditor/LastWorkspace", &perspective);
        m_auimgr178->LoadPerspective(perspective);
    }
    m_auimgr178->GetPane(m_tileSetPropertiesPanel).Dock();
    m_auimgr178->GetPane(m_tileEditorPanel).Dock();
    m_auimgr178->Update();

    m_tileSetToolBar->ToggleTool(EDIT_TILE_TOOL_ID, m_auimgr178->GetPane(m_tileEditorPanel).IsShown());

    if(tileSet.textureName == "")
    {
        //This is probably a new object, open the tileset config dialog
        wxCommandEvent fakeEvent;
        OnTileSetConfigureButtonClicked(fakeEvent);
    }

    UpdateLayerChoice();
}

TileMapObjectEditor::~TileMapObjectEditor()
{
    wxConfigBase::Get()->Write("/TileMapObjectEditor/LastWorkspace", m_auimgr178->SavePerspective());
}

void TileMapObjectEditor::OnLayerChoiceChanged(wxCommandEvent& event)
{
    m_tileMapPanel->SetCurrentLayer(m_layerChoice->GetSelection());
}

void TileMapObjectEditor::OnHideUpperLayerChecked(wxCommandEvent& event)
{
    m_tileMapPanel->HideUpperLayers(m_mainPanelToolbar->GetToolState(HIDE_UPPER_LAYERS_TOOL_ID));
}

void TileMapObjectEditor::OnTileSetSelectionChanged(TileSelectionEvent &event)
{
    m_tileMapPanel->OnTileSetSelectionChanged(event);
    m_tileEditorPanel->OnTileSetSelectionChanged(event);
}

void TileMapObjectEditor::UpdateLayerChoice()
{
    m_layerChoice->SetSelection(m_tileMapPanel->GetCurrentLayer());
}

bool TileMapObjectEditor::AskCloseConfirmation()
{
    return wxMessageBox(_("Do you want to cancel all the changes made ?"), _("Confirm"), wxYES_NO|wxICON_QUESTION|wxNO_DEFAULT, this) == wxYES;
}

void TileMapObjectEditor::OnCancelButtonPressed(wxCommandEvent& event)
{
    if(AskCloseConfirmation())
        EndModal(0);
}

void TileMapObjectEditor::OnOkButtonPressed(wxCommandEvent& event)
{
    object.tileSet = TileSetProxy(tileSet);
    object.tileMap = TileMapProxy(tileMap);

    EndModal(1);
}

void TileMapObjectEditor::OnTileSetConfigureButtonClicked(wxCommandEvent& event)
{
    std::string oldTextureName = tileSet.textureName;

    TileSetConfigurationEditor dialog(this, tileSet, game, mainFrameWrapper);
    dialog.ShowModal();

    tileSet.LoadResources(game);
    tileSet.Generate();

    if(oldTextureName != tileSet.textureName)
    {
        //Need to reset the hitboxes
        tileSet.ResetHitboxes();
    }

    m_tileSetPanel->Update();
    m_tileMapPanel->Update();
}

void TileMapObjectEditor::OnTileEditToolClicked(wxCommandEvent& event)
{
    m_auimgr178->GetPane(m_tileEditorPanel).Show(m_tileSetToolBar->GetToolState(EDIT_TILE_TOOL_ID));
    m_auimgr178->Update();
}

void TileMapObjectEditor::OnChangeMapSizeButtonClicked(wxCommandEvent& event)
{
    TileMapConfigurationEditor dialog(this, tileMap);
    dialog.ShowModal();

    m_tileMapPanel->Update();
}

void TileMapObjectEditor::OnHelpButtonClicked(wxHyperlinkEvent& event)
{
    gd::HelpFileAccess::Get()->OpenURL(_("http://www.wiki.compilgames.net/doku.php/en/game_develop/documentation/manual/built_tilemap"));
}

void TileMapObjectEditor::OnCloseButtonClicked(wxCloseEvent& event)
{
    if(AskCloseConfirmation())
        EndModal(0);
}

void TileMapObjectEditor::OnTileInsertionModeChanged(wxCommandEvent& event)
{
    if(event.GetId() == MODE_SINGLE_TILE_TOOL_ID)
    {
        m_tileMapPanel->SetInsertionMode(TileMapPanel::PencilMode);
    }
    else if(event.GetId() == MODE_RECTANGLE_TILE_TOOL_ID)
    {
        m_tileMapPanel->SetInsertionMode(TileMapPanel::RectangleMode);
    }
}

void TileMapObjectEditor::OnClearLayerToolClicked(wxCommandEvent& event)
{
    if(wxMessageBox(_("Are you sure you want to clear the whole layer ?"), _("Confirm"), wxYES_NO|wxICON_QUESTION|wxNO_DEFAULT, this) == wxYES)
        m_tileMapPanel->FillLayer(m_tileMapPanel->GetCurrentLayer(), -1);
}

void TileMapObjectEditor::OnFillLayerToolClicked(wxCommandEvent& event)
{
    if(wxMessageBox(_("Are you sure you want to fill the whole layer with that tile ?"), _("Confirm"), wxYES_NO|wxICON_QUESTION|wxNO_DEFAULT, this) == wxYES)
        m_tileMapPanel->FillLayer(m_tileMapPanel->GetCurrentLayer(), m_tileSetPanel->GetSelectedTile());
}

#endif
