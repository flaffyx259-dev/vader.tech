#include "ImGuiRenderer.h"
#include "IMGAY/imgui_internal.h"
#include "IMGAY/impl/imgui_impl_dx9.h"
#include "IMGAY/impl/imgui_impl_win32.h"
#include "IMGAY/imgui_stdlib.h"
#include "../Utils/Math.h"

extern Vector AutoPeekPos;

#define max(a, b) (((a) > (b)) ? (a) : (b))

ImColor u32(Color col)
{
	return ImColor(col.r(), col.g(), col.b(), col.a());
}

void ImGuiRendering::BeginScene()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::Begin(XorStr("##Backbuffer"), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

	ImGui::SetWindowPos(ImVec2(0, 0), 0);
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), 0);


	ImGuiWindow* window = ImGui::GetCurrentWindow();


	static int w, h;
	Interfaces::m_pEngine->GetScreenSize(w, h);

	static auto alpha = 0;
	auto speed = 800.0f * Interfaces::m_pGlobalVars->frametime;

	if (Interfaces::m_pEngine->IsInGame() && Interfaces::m_pEngine->IsConnected())
	{

		//IEsp::Get()->RenderImGuiNades();

		{ // autopeek circle
			bool condition = g_Vars.misc.autopeek && !AutoPeekPos.IsZero() && g_Vars.misc.autopeek_bind.enabled;

			static Vector last_autopeek_pos = AutoPeekPos;
			if (!AutoPeekPos.IsZero()) {
				last_autopeek_pos = AutoPeekPos;
			}

			if (condition) {
				Vector position = AutoPeekPos.IsZero() ? last_autopeek_pos : AutoPeekPos;
				float rad = max(2, 24) - 1.f;
				for (int i = 1; i < max(2, 24); i++) {
					if (rad > 2.f)
						DrawRing3D(position.x, position.y, position.z, round(rad), 180, Color(0, 0, 0, 0), Color(g_Vars.misc.autopeek_color.ToRegularColor().r(), g_Vars.misc.autopeek_color.ToRegularColor().g(), g_Vars.misc.autopeek_color.ToRegularColor().b(), i), 0, 0.f, false);

					rad -= 1.f;
				}
			}
		}
	}
}

void ImGuiRendering::EndScene()
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->PushClipRectFullScreen();

	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
}

void ImGuiRendering::arc(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->PathArcTo(ImVec2(x, y), radius, DEG2RAD(min_angle), DEG2RAD(max_angle), 32);
	window->DrawList->PathStroke(ImColor(col.r(), col.g(), col.b(), col.a()), false, thickness);
}

void ImGuiRendering::two_sided_arc(float x, float y, float radius, float scale, Color col, float thickness) {
	arc(x, y, radius, -270, -90 - 90 * (scale * 2), col, thickness);
	arc(x, y, radius, -90 + 90 * (scale * 2), 90, col, thickness);
}


void ImGuiRendering::DrawRing3D(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float prog, bool fill_prog)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	static float Step = (M_PI * 2.0f) / (points);
	std::vector<ImVec2> point;
	for (float lat = 0.f; lat <= M_PI * 2.0f; lat += Step)
	{
		const auto& point3d = Vector(sin(lat), cos(lat), 0.f) * radius;
		Vector2D point2d;
		if (Render::Engine::WorldToScreen(Vector(x, y, z) + point3d, point2d))
			point.push_back(ImVec2(point2d.x, point2d.y));
	}
	window->DrawList->AddConvexPolyFilled(point.data(), point.size(), u32(color2));
	window->DrawList->AddPolyline(point.data(), max(0, point.size() * prog), u32(color1), prog == 1.f, thickness);
}

void ImGuiRendering::DrawCollisionRing3D(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float prog)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	static auto prevScreenPos = Vector(0.0f, 0.0f, 0.0f);
	static auto step = M_PI * 2.0f / points;
	Vector origin = Vector(x, y, z);
	auto screenPos = Vector2D(0.0f, 0.0f);
	std::vector<ImVec2> point;
	for (float rotation = 0.0f; rotation <= M_PI * 2.0f; rotation += step) //-V1034
	{
		Vector pos = Vector(x, y, z) + (Vector(sin(rotation), cos(rotation), 0.f) * radius);

		Ray_t ray;
		CGameTrace trace;
		CTraceFilterWorldOnly filter;

		ray.Init(origin, pos);
		Interfaces::m_pEngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace);

		Ray_t ray2;
		CTraceFilterWorldAndPropsOnly filter2;
		CGameTrace trace2;

		ray2.Init(trace.endpos, trace.endpos - Vector(0.0f, 0.0f, 140.0f));
		Interfaces::m_pEngineTrace->TraceRay(ray2, MASK_SHOT_BRUSHONLY, &filter2, &trace2);
		Vector real_end;
		if (trace2.fraction < 1.0f)
			real_end = trace2.endpos + Vector(0.0f, 0.0f, 2.0f);
		if (Render::Engine::WorldToScreen(real_end, screenPos))
		{
			if (!prevScreenPos.IsZero())
				point.push_back(ImVec2(screenPos.x, screenPos.y));
			prevScreenPos = Vector(screenPos.x, screenPos.y, 0.0f);
		}
	}

	window->DrawList->AddConvexPolyFilled(point.data(), point.size(), u32(color2));
	window->DrawList->AddPolyline(point.data(), max(0, point.size() * prog), u32(color1), prog == 1.f, thickness);
}

void ImGuiRendering::DrawSphere(Vector origin, float radius, float angle, float scale, Color color) {
	std::vector< ImVec2 > vertices{};
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	float step = (1.f / radius) + DEG2RAD(angle);

	for (float lat = 0; lat < (M_PI * scale); lat += step) {
		// reset.
		vertices.clear();

		for (float lon{}; lon < M_PI; lon += step) {
			Vector point{
				origin.x + (radius * std::sin(lat) * std::cos(lon)),
				origin.y + (radius * std::sin(lat) * std::sin(lon)),
				origin.z + (radius * std::cos(lat))
			};

			Vector2D screen;
			if (Render::Engine::WorldToScreen(point, screen))
				vertices.emplace_back(ImVec2(screen.x, screen.y));
		}

		if (vertices.empty())
			continue;

		window->DrawList->AddPolyline(vertices.data(), max(0, vertices.size()), u32(color), true, 1);
	}
}

void ImGuiRendering::DrawDepthRing3D(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float prog)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	static auto prevScreenPos = Vector(0.0f, 0.0f, 0.0f);
	static auto step = M_PI * 2.0f / points;
	Vector origin = Vector(x, y, z);
	auto screenPos = Vector2D(0.0f, 0.0f);
	std::vector<ImVec2> point;
	for (float rotation = 0.0f; rotation <= M_PI * 2.0f; rotation += step)
	{
		Vector pos = Vector(x, y, z) + (Vector(sin(rotation), cos(rotation), 0.f) * radius);

		Ray_t ray2;
		CTraceFilterWorldAndPropsOnly filter2;
		CGameTrace trace2;

		ray2.Init(pos, pos - Vector(0.0f, 0.0f, 90.0f));
		Interfaces::m_pEngineTrace->TraceRay(ray2, MASK_SHOT_BRUSHONLY, &filter2, &trace2);
		Vector real_end = pos;
		if (trace2.fraction < 1.0f)
		{
			real_end = trace2.endpos + Vector(0.0f, 0.0f, 2.0f);
		}
		if (Render::Engine::WorldToScreen(real_end, screenPos))
		{
			if (!prevScreenPos.IsZero())
				point.push_back(ImVec2(screenPos.x, screenPos.y));
			prevScreenPos = Vector(screenPos.x, screenPos.y, 0.0f);
		}
	}

	window->DrawList->AddConvexPolyFilled(point.data(), point.size(), u32(color2));
	window->DrawList->AddPolyline(point.data(), max(0, point.size() * prog), u32(color1), prog == 1.f, thickness);
}

void ImGuiRendering::PolyLine(const ImVec2* points, const int points_count, Color clr, bool closed, float thickness, Color fill)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->AddConvexPolyFilled(points, points_count, u32(fill));
	window->DrawList->AddPolyline(points, points_count, u32(clr), closed, thickness);
}

void ImGuiRendering::DrawCircle(float x, float y, float r, float s, Color color)
{
	float Step = Math::pi * 2.0 / s;
	for (float a = 0; a < (Math::pi * 2.0); a += Step)
	{
		float x1 = r * cos(a) + x;
		float y1 = r * sin(a) + y;
		float x2 = r * cos(a + Step) + x;
		float y2 = r * sin(a + Step) + y;

		DrawLine(x1, y1, x2, y2, color);
	}
}

struct CUSTOMVERTEX {
	FLOAT x, y, z;
	FLOAT rhw;
	DWORD color;
};

void ImGuiRendering::DrawString(float x, float y, Color color, int flags, ImFont* font, const char* message, ...)
{
	char output[1024] = {};
	va_list args;
	va_start(args, message);
	vsprintf_s(output, message, args);
	va_end(args);

	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->PushTextureID(font->ContainerAtlas->TexID);
	ImGui::PushFont(font);
	auto coord = ImVec2(x, y);
	auto size = ImGui::CalcTextSize(output);
	auto coord_out = ImVec2{ coord.x + 1.f, coord.y + 1.f };
	Color outline_clr = Color(0, 0, 0, color.a());

	int width = 0, height = 0;

	if (!(flags & render2::centered_x))
		size.x = 0;
	if (!(flags & render2::centered_y))
		size.y = 0;

	ImVec2 pos = ImVec2(coord.x - (size.x * .5), coord.y - (size.y * .5));

	if (flags & render2::outline)
	{
		pos.y++;
		window->DrawList->AddText(pos, u32(outline_clr), output);
		pos.x++;
		window->DrawList->AddText(pos, u32(outline_clr), output);
		pos.y--;
		window->DrawList->AddText(pos, u32(outline_clr), output);
		pos.x--;
		window->DrawList->AddText(pos, u32(outline_clr), output);
	}

	window->DrawList->AddText(pos, u32(color), output);
	ImGui::PopFont();
}


void ImGuiRendering::DrawString2(float x, float y, Color color, int flags, ImFont* font, const std::string message, ...)
{
	char output[1024] = {};
	va_list args;
	va_start(args, message);
	vsprintf_s(output, message.c_str(), args);
	va_end(args);

	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->PushTextureID(font->ContainerAtlas->TexID);
	ImGui::PushFont(font);
	auto coord = ImVec2(x, y);
	auto size = ImGui::CalcTextSize(output);
	auto coord_out = ImVec2{ coord.x + 1.f, coord.y + 1.f };
	Color outline_clr = Color(0, 0, 0, color.a());

	int width = 0, height = 0;

	if (!(flags & render2::centered_x))
		size.x = 0;
	if (!(flags & render2::centered_y))
		size.y = 0;

	ImVec2 pos = ImVec2(coord.x - (size.x * .5), coord.y - (size.y * .5));

	if (flags & render2::outline)
	{
		pos.y++;
		window->DrawList->AddText(pos, ImColor(outline_clr.r(), outline_clr.g(), outline_clr.b(), outline_clr.a()), output);
		pos.x++;
		window->DrawList->AddText(pos, ImColor(outline_clr.r(), outline_clr.g(), outline_clr.b(), outline_clr.a()), output);
		pos.y--;
		window->DrawList->AddText(pos, ImColor(outline_clr.r(), outline_clr.g(), outline_clr.b(), outline_clr.a()), output);
		pos.x--;
		window->DrawList->AddText(pos, ImColor(outline_clr.r(), outline_clr.g(), outline_clr.b(), outline_clr.a()), output);
	}

	window->DrawList->AddText(pos, u32(color), output);
	ImGui::PopFont();
}


void ImGuiRendering::DrawEspBox(Vector leftUpCorn, Vector rightDownCorn, Color clr, float width)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	ImVec2 min = ImVec2(leftUpCorn.x, leftUpCorn.y);
	ImVec2 max = ImVec2(rightDownCorn.x, rightDownCorn.y);
	window->DrawList->AddRect(min, max, u32(clr), 0.0F, -1, width);
}

void ImGuiRendering::DrawLine(float x1, float y1, float x2, float y2, Color clr, float thickness)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), u32(clr), thickness);
}

void ImGuiRendering::DrawLineGradient(float x1, float y1, float x2, float y2, Color clr1, Color cl2, float thickness, bool flip)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (!flip)
		window->DrawList->AddRectFilledMultiColor(ImVec2(x1, y1), ImVec2(x1 + x2, y1 + y2), u32(clr1), u32(cl2), u32(cl2), u32(clr1));
	else
		window->DrawList->AddRectFilledMultiColor(ImVec2(x1, y1), ImVec2(x1 + x2, y1 + y2), u32(clr1), u32(clr1), u32(cl2), u32(cl2));
}

void ImGuiRendering::filled_rect_world(Vector center, Vector2D size, Color color, int angle, float damage) {
	Vector add_top_left, add_top_right, add_bot_left, add_bot_right;
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	switch (angle) {
	case 0: // Z
		add_top_left = Vector(-size.x, -size.y, 0);
		add_top_right = Vector(size.x, -size.y, 0);

		add_bot_left = Vector(-size.x, size.y, 0);
		add_bot_right = Vector(size.x, size.y, 0);

		break;
	case 1: // Y
		add_top_left = Vector(-size.x, 0, -size.y);
		add_top_right = Vector(size.x, 0, -size.y);

		add_bot_left = Vector(-size.x, 0, size.y);
		add_bot_right = Vector(size.x, 0, size.y);

		break;
	case 2: // X
		add_top_left = Vector(0, -size.y, -size.x);
		add_top_right = Vector(0, -size.y, size.x);

		add_bot_left = Vector(0, size.y, -size.x);
		add_bot_right = Vector(0, size.y, size.x);

		break;
	}

	//top line
	Vector c_top_left = center + add_top_left;
	Vector c_top_right = center + add_top_right;

	//bottom line
	Vector c_bot_left = center + add_bot_left;
	Vector c_bot_right = center + add_bot_right;

	Vector2D top_left, top_right, bot_left, bot_right;
	if (Render::Engine::WorldToScreen(c_top_left, top_left) &&
		Render::Engine::WorldToScreen(c_top_right, top_right) &&
		Render::Engine::WorldToScreen(c_bot_left, bot_left) &&
		Render::Engine::WorldToScreen(c_bot_right, bot_right)) {

		std::vector<ImVec2> vertices;

		vertices.push_back(ImVec2(top_left.x, top_left.y));
		vertices.push_back(ImVec2(top_right.x, top_right.y));
		vertices.push_back(ImVec2(bot_right.x, bot_right.y));
		vertices.push_back(ImVec2(bot_left.x, bot_left.y));

		window->DrawList->AddConvexPolyFilled(vertices.data(), vertices.size(), u32(color));

	}
}

void ImGuiRendering::Rect(float x, float y, float w, float h, Color clr, float rounding, ImDrawCornerFlags flags)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), u32(clr), rounding, flags);
}

void ImGuiRendering::FilledRect(float x, float y, float w, float h, Color clr, float rounding, ImDrawCornerFlags flags)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), u32(clr), rounding, flags);
}

void ImGuiRendering::FilledRect2(float x, float y, float w, float h, Color clr, float rounding, ImDrawCornerFlags flags)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddRectFilled(ImVec2(x, y), ImVec2(w, h), u32(clr), rounding, flags);
}

void ImGuiRendering::Triangle(float x1, float y1, float x2, float y2, float x3, float y3, Color clr, float thickness)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddTriangle(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), u32(clr), thickness);
}

void ImGuiRendering::TriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3, Color clr)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), u32(clr));
}

void ImGuiRendering::QuadFilled(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, Color clr)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddQuadFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), ImVec2(x4, y4), u32(clr));
}

void ImGuiRendering::CircleFilled(float x1, float y1, float radius, Color col, int segments)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddCircleFilled(ImVec2(x1, y1), radius, u32(col), segments);
}

void ImGuiRendering::DrawWave(Vector loc, float radius, Color color, float thickness)
{
	static float Step = Math::pi * 3.0f / 40;
	Vector2D prev;
	for (float lat = 0; lat <= Math::pi * 3.0f; lat += Step)
	{
		float sin1 = sin(lat);
		float cos1 = cos(lat);
		float sin3 = sin(0.0f);
		float cos3 = cos(0.0f);

		Vector point1;
		point1 = Vector(sin1 * cos3, cos1, sin1 * sin3) * radius;
		Vector point3 = loc;
		Vector2D Out;
		point3 += point1;

		if (Render::Engine::WorldToScreen(point3, Out))
		{
			if (lat > 0.000f)
				DrawLine(prev.x, prev.y, Out.x, Out.y, color, thickness);
		}
		prev = Out;
	}
}