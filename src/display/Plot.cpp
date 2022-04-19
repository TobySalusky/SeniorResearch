//
// Created by Tobiathan on 4/11/22.
//

#include "Plot.h"
#include "../program/Program.h"
#include "../animation/Timeline.h"
#include "../util/Controls.h"
#include "../screens/MainScreen.h"

Plot::Plot(const GLWindow &window) :
	graphScene(window.GetBufferWidth(), window.GetBufferHeight()) {}

void Plot::Update(const Project &project, float deltaTime) {
	onScreen = Util::NormalizeToRectNPFlipped(Program::GetInput().GetMouse(), plotRect);

	project.GetCurrentModelObject()->EditMakeup({editContext, Program::GetInput(), drawMode, onScreen, graphFocused, graphView, plotRect});
	graphView.Update({Program::GetInput(), plotRect});
}

void Plot::PostUpdate(const Project& project, float deltaTime) {
	HandleUndoing();
	editContext.Update(deltaTime); // NOTE: not sure why these must be post? look into it, may be pointless
}

void Plot::Render(const Project& project) {
	RenderTarget::Bind(graphScene);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader2D.Enable();

	shader2D.SetModel(graphView.GenProjection());
	plot.SetLineScale(graphView.scale);

	plot.AddQuad({-1.0f, 0.0f}, {1.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f});
	graphView.Render({plot, Program::GetInput()});

	Timeline& timeline = MainScreen::GetComponents().timeline;
	if (!timeline.IsPlaying()) timeline.RenderOnionSkin(plot, drawMode);

	project.GetCurrentModelObject()->Render2D({plot, drawMode, onScreen, graphView, editContext, Program::GetInput()});

	plot.ImmediateClearingRender();

	shader2D.Disable();
	RenderTarget::Unbind();

}

void Plot::Gui() {
	ImGui::Begin("Graph Scene");

	Vec2 displayDimens =
			Util::ToVec(ImGui::GetWindowContentRegionMax()) - Util::ToVec(ImGui::GetWindowContentRegionMin());
	static Vec2 lastDisplayDimens;
	lastDisplayDimens = displayDimens;
	ImGui::ImageButton((void *) (intptr_t) graphScene.GetTexture(), Util::ToImVec(displayDimens), {0.0f, 1.0f},
	                   {1.0f, 0.0f}, 0);

	plotRect = ImGuiHelper::ItemRectRemovePadding(0.0f, 0.0f);
	// TODO: IsItemActive works perfectly for mouse, but focus works better for keyboard :/
	graphFocused = ImGui::IsItemFocused();

	ImGuiHelper::InnerWindowBorders();

	ImGui::End();
}

void Plot::HandleUndoing() {
	if (!editContext.CanUndo()) return;

	bool initUndo = Controls::Check(CONTROLS_Undo);
	bool holdUndo = Controls::Check(CONTROLS_UndoHold) && editContext.CanUndoHold();
	if (initUndo || holdUndo) {
		editContext.UseUndo(initUndo);
		Undos::UseLast();
	}
}

void Plot::HotKeys() {
	if (Controls::Check(CONTROLS_SetLayerPrimary)) drawMode = Enums::MODE_PLOT;
	if (Controls::Check(CONTROLS_SetLayerSecondary)) drawMode = Enums::MODE_GRAPH_Y;
	if (Controls::Check(CONTROLS_SetLayerTertiary)) drawMode = Enums::MODE_GRAPH_Z;
	if (Controls::Check(CONTROLS_SetLayerQuaternary)) drawMode = Enums::MODE_CROSS_SECTION;
}

void Plot::ToolbarGui(Project& project) {
	// Toolbar
	ImGui::Begin("Toolbar");
	{
		project.GetCurrentModelObject()->ModeSet("L1", Enums::DrawMode::MODE_PLOT, drawMode);
		project.GetCurrentModelObject()->ModeSet("L2", Enums::DrawMode::MODE_GRAPH_Y, drawMode);
		project.GetCurrentModelObject()->ModeSet("L3", Enums::DrawMode::MODE_GRAPH_Z, drawMode);
		project.GetCurrentModelObject()->ModeSet("L4", Enums::DrawMode::MODE_CROSS_SECTION, drawMode);

		ImGuiHelper::InnerWindowBorders();
	}
	ImGui::End();
}
