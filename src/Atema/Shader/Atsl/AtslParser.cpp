/*
	Copyright 2022 Jordi SUBIRANA

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
#include <Atema/Shader/Atsl/AtslUtils.hpp>
#include <Atema/Core/Error.hpp>

using namespace at;

namespace
{
	enum class AtslNumberSuffix
	{
		Unspecified,
		Unsigned,
		Float,
	};

	AtslNumberSuffix getNumberSuffix(char c)
	{
		switch (c)
		{
			case 'u':
			case 'U':
				return AtslNumberSuffix::Unsigned;
			case 'f':
			case 'F':
				return AtslNumberSuffix::Float;
			default:
				break;
		}

		return AtslNumberSuffix::Unspecified;
	}

	bool isAlphabetic(char c)
	{
		return std::isalpha(c) || c == '_';
	}

	bool isDigit(char c)
	{
		return std::isdigit(c);
	}

	bool isHexadecimal(char c)
	{
		return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
	}

	bool isNumberSuffix(char c)
	{
		return getNumberSuffix(c) != AtslNumberSuffix::Unspecified;
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
			case '?':
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

	bool isCommentSymbol(char c)
	{
		return c == '/';
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
	
	m_currentLine = 1;
	m_currentColumn = 1;

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

			continue;
		}
		// New line
		else if (isNewLine(c))
		{
			m_currentLine++;
			m_currentColumn = 0; // Will be incremented by the next char

			continue;
		}
		// Symbols
		else if (isSymbol(c))
		{
			if (isCommentSymbol(c) && hasNext())
			{
				// Check if this is a comment : if it is, go to the next line
				if (isCommentSymbol(getNext()))
				{
					while (hasNext())
					{
						if (isNewLine(getNext()))
							break;
					}

					m_currentLine++;
					m_currentColumn = 0; // Will be incremented by the next char

					continue;
				}
				else
				{
					gotoLastChar();
				}
			}

			tokens.emplace_back(atsl::getSymbol(c));
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
				tokens.emplace_back(getBooleanValue(identifier));
			}
			else if (atsl::isKeyword(identifier))
			{
				tokens.emplace_back(atsl::getKeyword(identifier));
			}
			else
			{
				tokens.emplace_back(identifier);
			}
		}
		// Numbers
		else if (isDigit(c))
		{
			std::string strValue;
			strValue += c;

			AtslNumberSuffix numberSuffix = AtslNumberSuffix::Unspecified;

			bool isFloat = false;
			bool isFloatExponent = false;

			// Check if this is an hexadecimal number
			bool isHex = false;
			int base = 10;
			if (c == '0')
			{
				const auto n = getNext();

				if (n == 'x' || n == 'X')
				{
					isHex = true;
					base = 16;
					strValue += n;
				}
				else
				{
					gotoLastChar();
				}
			}

			// Get full number
			while (hasNext())
			{
				const auto n = getNext();

				if (isDigit(n))
				{
					strValue += n;
				}
				else if (!isFloatExponent)
				{
					if (isHex && isHexadecimal(n))
					{
						strValue += n;
					}
					else if (n == '.')
					{
						if (isHex)
						{
							ATEMA_ERROR("Invalid '.' character in hex number");
						}

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
					else if (n == 'e' || n == 'E')
					{
						strValue += n;

						isFloat = true;
						isFloatExponent = true; // For now we only look for digits

						const char sign = getNext();

						if (sign == '-' || sign == '+')
							strValue += sign;
						else
							gotoLastChar();
					}
					else if (isNumberSuffix(n))
					{
						numberSuffix = getNumberSuffix(n);

						break;
					}
					else
					{
						// The char was not part of the number : get back !
						gotoLastChar();
						break;
					}
				}
				else if (isNumberSuffix(n))
				{
					numberSuffix = getNumberSuffix(n);

					break;
				}
				else
				{
					// The char was not part of the number : get back !
					gotoLastChar();
					break;
				}
			}

			if (isFloat || numberSuffix == AtslNumberSuffix::Float)
			{
				tokens.emplace_back(std::stof(strValue));
			}
			else if (numberSuffix == AtslNumberSuffix::Unsigned)
			{
				tokens.emplace_back(static_cast<uint32_t>(std::stoll(strValue, nullptr, base)));
			}
			else
			{
				tokens.emplace_back(static_cast<int32_t>(std::stoll(strValue, nullptr, base)));
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
	return m_currentIndex < m_code.size();
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
