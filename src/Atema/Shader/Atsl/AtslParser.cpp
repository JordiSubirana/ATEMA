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

#include <Atema/Shader/Atsl/AtslParser.hpp>
#include <Atema/Shader/Atsl/AtslToken.hpp>
#include <Atema/Shader/Atsl/Utils.hpp>
#include <Atema/Core/Error.hpp>

using namespace at;

namespace
{
	bool isAlphabetic(char c)
	{
		return std::isalpha(c) || c == '_';
	}

	bool isDigit(char c)
	{
		return std::isdigit(c);
	}

	bool isAlphaNumeric(char c)
	{
		return isAlphabetic(c) || isDigit(c);
	}

	bool isSymbol(char c)
	{
		switch (c)
		{
			case '+':
			case '-':
			case '*':
			case '/':
			case '^':
			case '%':
			case '=':
			case '<':
			case '>':
			case '&':
			case '|':
			case '!':
			case '.':
			case ',':
			case ':':
			case ';':
			case '{':
			case '}':
			case '[':
			case ']':
			case '(':
			case ')':
				return true;
			default:
				break;
		}

		return false;
	}

	bool isSpace(char c)
	{
		switch (c)
		{
			case ' ':
			case '\t':
			case '\r':
				return true;
			default:
				break;
		}

		return false;
	}

	bool isNewLine(char c)
	{
		return c == '\n';
	}

	bool isBooleanValue(const std::string& str)
	{
		return str == "false" || str == "true";
	}

	bool getBooleanValue(const std::string& str)
	{
		if (str == "false")
			return false;

		return true;
	}
}

AtslParser::AtslParser()
{
}

AtslParser::~AtslParser()
{
}

std::vector<AtslToken> AtslParser::createTokens(const std::string& code)
{
	// Initialize variables
	m_code = code;
	
	m_currentLine = 0;
	m_currentColumn = 0;

	m_currentIndex = 0;

	// Create tokens
	std::vector<AtslToken> tokens;

	while (hasNext())
	{
		const auto line = m_currentLine;
		const auto column = m_currentColumn;

		const auto c = getNext();

		// Space
		if (isSpace(c))
		{
			m_currentColumn++;
		}
		// New line
		else if (isNewLine(c))
		{
			m_currentLine++;
			m_currentColumn = -1; // Will be incremented by the next char
		}
		// Symbols
		else if (isSymbol(c))
		{
			tokens.push_back({ atsl::getSymbol(c) });
		}
		// Alphabetic
		else if (isAlphabetic(c))
		{
			AtslIdentifier identifier;
			identifier += c;

			// Get full word
			while (hasNext())
			{
				const auto n = getNext();

				if (isAlphaNumeric(n))
				{
					identifier += n;
				}
				else
				{
					// The char was not part of the word : get back !
					gotoLastChar();
					break;
				}
			}

			if (isBooleanValue(identifier))
			{
				tokens.push_back({ getBooleanValue(identifier) });
			}
			else if (atsl::isKeyword(identifier))
			{
				tokens.push_back({ atsl::getKeyword(identifier) });
			}
			else
			{
				tokens.push_back({ identifier });
			}
		}
		// Numbers
		else if (isDigit(c))
		{
			std::string strValue;
			strValue += c;

			bool isFloat = false;

			// Get full number
			while (hasNext())
			{
				const auto n = getNext();

				if (isDigit(n))
				{
					strValue += n;
				}
				else if (n == '.')
				{
					// Check if it was already a float
					if (isFloat)
					{
						// The char was not part of the number : get back !
						gotoLastChar();
						break;
					}

					strValue += n;

					isFloat = true;
				}
				else
				{
					// The char was not part of the number : get back !
					gotoLastChar();
					break;
				}
			}

			if (isFloat)
			{
				tokens.push_back({ std::stof(strValue) });
			}
			else
			{
				tokens.push_back({ static_cast<uint32_t>(std::stoul(strValue)) });
			}
		}
		// End of file
		else if (!hasNext())
		{
			break;
		}
		// Invalid case
		else
		{
			ATEMA_ERROR("Unsupported character");

			return tokens;
		}

		auto& token = tokens.back();
		token.line = line;
		token.column = static_cast<size_t>(column);
	}

	return tokens;
}

bool AtslParser::hasNext() const
{
	return m_currentIndex < m_code.size() - 1;
}

char AtslParser::getNext()
{
	ATEMA_ASSERT(hasNext(), "No next character available");

	m_currentColumn++;

	return m_code[m_currentIndex++];
}

void AtslParser::gotoLastChar()
{
	m_currentIndex--;
	m_currentColumn--;
}
