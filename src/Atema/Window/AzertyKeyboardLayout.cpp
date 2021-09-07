/*
	Copyright 2021 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Atema/Window/AzertyKeyboardLayout.hpp>

#include <array>

using namespace at;

namespace
{
	const std::array<Key, 256> values =
	{
		/* 000 */ Key::Unknown,
		/* 001 */ Key::Unknown,
		/* 002 */ Key::Unknown,
		/* 003 */ Key::Unknown,
		/* 004 */ Key::Unknown,
		/* 005 */ Key::Unknown,
		/* 006 */ Key::Unknown,
		/* 007 */ Key::Unknown,
		/* 008 */ Key::Unknown,
		/* 009 */ Key::Unknown,
		/* 010 */ Key::Unknown,
		/* 011 */ Key::Unknown,
		/* 012 */ Key::Unknown,
		/* 013 */ Key::Unknown,
		/* 014 */ Key::Unknown,
		/* 015 */ Key::Unknown,
		/* 016 */ Key::Unknown,
		/* 017 */ Key::Unknown,
		/* 018 */ Key::Unknown,
		/* 019 */ Key::Unknown,
		/* 020 */ Key::Unknown,
		/* 021 */ Key::Unknown,
		/* 022 */ Key::Unknown,
		/* 023 */ Key::Unknown,
		/* 024 */ Key::Unknown,
		/* 025 */ Key::Unknown,
		/* 026 */ Key::Unknown,
		/* 027 */ Key::Unknown,
		/* 028 */ Key::Unknown,
		/* 029 */ Key::Unknown,
		/* 030 */ Key::Unknown,
		/* 031 */ Key::Unknown,
		/* 032 */ Key::Space,
		/* 033 */ Key::Slash,
		/* 034 */ Key::Key3,
		/* 035 */ Key::Key3,
		/* 036 */ Key::RightBracket,
		/* 037 */ Key::Apostrophe,
		/* 038 */ Key::Key1,
		/* 039 */ Key::Key4,
		/* 040 */ Key::Key5,
		/* 041 */ Key::Minus,
		/* 042 */ Key::Backslash,
		/* 043 */ Key::Equal,
		/* 044 */ Key::M,
		/* 045 */ Key::Key6,
		/* 046 */ Key::Comma,
		/* 047 */ Key::Period,
		/* 048 */ Key::Key0,
		/* 049 */ Key::Key1,
		/* 050 */ Key::Key2,
		/* 051 */ Key::Key3,
		/* 052 */ Key::Key4,
		/* 053 */ Key::Key5,
		/* 054 */ Key::Key6,
		/* 055 */ Key::Key7,
		/* 056 */ Key::Key8,
		/* 057 */ Key::Key9,
		/* 058 */ Key::Period,
		/* 059 */ Key::Comma,
		/* 060 */ Key::Backslash,
		/* 061 */ Key::Equal,
		/* 062 */ Key::Backslash,
		/* 063 */ Key::M,
		/* 064 */ Key::Minus,
		/* 065 */ Key::Q,
		/* 066 */ Key::B,
		/* 067 */ Key::C,
		/* 068 */ Key::D,
		/* 069 */ Key::E,
		/* 070 */ Key::F,
		/* 071 */ Key::G,
		/* 072 */ Key::H,
		/* 073 */ Key::I,
		/* 074 */ Key::J,
		/* 075 */ Key::K,
		/* 076 */ Key::L,
		/* 077 */ Key::Unknown, // ':'
		/* 078 */ Key::N,
		/* 079 */ Key::O,
		/* 080 */ Key::P,
		/* 081 */ Key::A,
		/* 082 */ Key::R,
		/* 083 */ Key::S,
		/* 084 */ Key::T,
		/* 085 */ Key::U,
		/* 086 */ Key::V,
		/* 087 */ Key::Z,
		/* 088 */ Key::X,
		/* 089 */ Key::Y,
		/* 090 */ Key::W,
		/* 091 */ Key::Key5,
		/* 092 */ Key::Key8,
		/* 093 */ Key::Minus,
		/* 094 */ Key::Key9,
		/* 095 */ Key::Key8,
		/* 096 */ Key::Key7,
		/* 097 */ Key::Q,
		/* 098 */ Key::B,
		/* 099 */ Key::C,

		/* 100 */ Key::D,
		/* 101 */ Key::E,
		/* 102 */ Key::F,
		/* 103 */ Key::G,
		/* 104 */ Key::H,
		/* 105 */ Key::I,
		/* 106 */ Key::J,
		/* 107 */ Key::K,
		/* 108 */ Key::L,
		/* 109 */ Key::Comma,
		/* 110 */ Key::N,
		/* 111 */ Key::O,
		/* 112 */ Key::P,
		/* 113 */ Key::A,
		/* 114 */ Key::R,
		/* 115 */ Key::S,
		/* 116 */ Key::T,
		/* 117 */ Key::U,
		/* 118 */ Key::V,
		/* 119 */ Key::Z,
		/* 120 */ Key::X,
		/* 121 */ Key::Y,
		/* 122 */ Key::W,
		/* 123 */ Key::Key4,
		/* 124 */ Key::Key6,
		/* 125 */ Key::Equal,
		/* 126 */ Key::Key2,
		/* 127 */ Key::Unknown,
		/* 128 */ Key::Unknown,
		/* 129 */ Key::Unknown,
		/* 130 */ Key::Backslash,
		/* 131 */ Key::Unknown,
		/* 132 */ Key::Unknown,
		/* 133 */ Key::Key0,
		/* 134 */ Key::Unknown,
		/* 135 */ Key::Key9,
		/* 136 */ Key::Unknown,
		/* 137 */ Key::Unknown,
		/* 138 */ Key::Key7,
		/* 139 */ Key::Unknown,
		/* 140 */ Key::Unknown,
		/* 141 */ Key::Unknown,
		/* 142 */ Key::Unknown,
		/* 143 */ Key::Unknown,
		/* 144 */ Key::Unknown,
		/* 145 */ Key::Unknown,
		/* 146 */ Key::Unknown,
		/* 147 */ Key::Unknown,
		/* 148 */ Key::Unknown,
		/* 149 */ Key::Unknown,
		/* 150 */ Key::Unknown,
		/* 151 */ Key::Unknown,
		/* 152 */ Key::Unknown,
		/* 153 */ Key::Unknown,
		/* 154 */ Key::Unknown,
		/* 155 */ Key::Unknown,
		/* 156 */ Key::RightBracket,
		/* 157 */ Key::Unknown,
		/* 158 */ Key::Unknown,
		/* 159 */ Key::Unknown,
		/* 160 */ Key::Unknown,
		/* 161 */ Key::World1,
		/* 162 */ Key::World2,
		/* 163 */ Key::Unknown,
		/* 164 */ Key::Unknown,
		/* 165 */ Key::Unknown,
		/* 166 */ Key::Unknown,
		/* 167 */ Key::Unknown,
		/* 168 */ Key::Unknown,
		/* 169 */ Key::Unknown,
		/* 170 */ Key::Unknown,
		/* 171 */ Key::Unknown,
		/* 172 */ Key::Unknown,
		/* 173 */ Key::Unknown,
		/* 174 */ Key::Unknown,
		/* 175 */ Key::Unknown,
		/* 176 */ Key::Unknown,
		/* 177 */ Key::Unknown,
		/* 178 */ Key::Unknown,
		/* 179 */ Key::Unknown,
		/* 180 */ Key::Unknown,
		/* 181 */ Key::Unknown,
		/* 182 */ Key::Unknown,
		/* 183 */ Key::Unknown,
		/* 184 */ Key::Unknown,
		/* 185 */ Key::Unknown,
		/* 186 */ Key::Unknown,
		/* 187 */ Key::Unknown,
		/* 188 */ Key::Unknown,
		/* 189 */ Key::Unknown,
		/* 190 */ Key::Unknown,
		/* 191 */ Key::Unknown,
		/* 192 */ Key::Unknown,
		/* 193 */ Key::Unknown,
		/* 194 */ Key::Unknown,
		/* 195 */ Key::Unknown,
		/* 196 */ Key::Unknown,
		/* 197 */ Key::Unknown,
		/* 198 */ Key::Unknown,
		/* 199 */ Key::Unknown,

		/* 200 */ Key::Unknown,
		/* 201 */ Key::Unknown,
		/* 202 */ Key::Unknown,
		/* 203 */ Key::Unknown,
		/* 204 */ Key::Unknown,
		/* 205 */ Key::Unknown,
		/* 206 */ Key::Unknown,
		/* 207 */ Key::Unknown,
		/* 208 */ Key::Unknown,
		/* 209 */ Key::Unknown,
		/* 210 */ Key::Unknown,
		/* 211 */ Key::Unknown,
		/* 212 */ Key::Unknown,
		/* 213 */ Key::Unknown,
		/* 214 */ Key::Unknown,
		/* 215 */ Key::Unknown,
		/* 216 */ Key::Unknown,
		/* 217 */ Key::Unknown,
		/* 218 */ Key::Unknown,
		/* 219 */ Key::Unknown,
		/* 220 */ Key::Unknown,
		/* 221 */ Key::Unknown,
		/* 222 */ Key::Unknown,
		/* 223 */ Key::Unknown,
		/* 224 */ Key::Unknown,
		/* 225 */ Key::Unknown,
		/* 226 */ Key::Unknown,
		/* 227 */ Key::Unknown,
		/* 228 */ Key::Unknown,
		/* 229 */ Key::Unknown,
		/* 230 */ Key::Unknown,
		/* 231 */ Key::Unknown,
		/* 232 */ Key::Unknown,
		/* 233 */ Key::Unknown,
		/* 234 */ Key::Unknown,
		/* 235 */ Key::Apostrophe,
		/* 236 */ Key::Unknown,
		/* 237 */ Key::Unknown,
		/* 238 */ Key::Unknown,
		/* 239 */ Key::Unknown,
		/* 240 */ Key::Unknown,
		/* 241 */ Key::Unknown,
		/* 242 */ Key::Unknown,
		/* 243 */ Key::Unknown,
		/* 244 */ Key::Unknown,
		/* 245 */ Key::Slash,
		/* 246 */ Key::Unknown,
		/* 247 */ Key::Unknown,
		/* 248 */ Key::Minus,
		/* 249 */ Key::Unknown,
		/* 250 */ Key::Unknown,
		/* 251 */ Key::Unknown,
		/* 252 */ Key::Unknown,
		/* 253 */ Key::Unknown,
		/* 254 */ Key::Unknown,
		/* 255 */ Key::Unknown
	};
}

AzertyKeyboardLayout::AzertyKeyboardLayout() :
	KeyboardLayout()
{
}

AzertyKeyboardLayout::~AzertyKeyboardLayout()
{
}

AzertyKeyboardLayout& AzertyKeyboardLayout::instance()
{
	static AzertyKeyboardLayout s_instance;

	return s_instance;
}

Key AzertyKeyboardLayout::getKey(uint8_t unicode) const noexcept
{
	return values[unicode];
}
