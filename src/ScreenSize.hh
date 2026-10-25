#pragma once

namespace Screen
{
	// setters
	void setWidth(int w) noexcept;
	void setHeight(int h) noexcept;

	// getters
	[[nodiscard]] int getWidth() noexcept;
	[[nodiscard]] int getHeight() noexcept;
}
