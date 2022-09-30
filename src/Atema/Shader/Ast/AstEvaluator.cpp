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

#include <Atema/Shader/Ast/AstEvaluator.hpp>
#include <Atema/Math/Math.hpp>

using namespace at;

namespace
{
	std::optional<bool> getBool(const ConstantValue& value)
	{
		if (value.is<bool>())
			return value.get<bool>();
		else if (value.is<int32_t>())
			return value.get<int32_t>() != static_cast<int32_t>(0);
		else if (value.is<uint32_t>())
			return value.get<uint32_t>() != static_cast<uint32_t>(0);
		else if (value.is<float>())
			return value.get<float>() != static_cast<float>(0);
		else if (value.is<Vector2i>())
			return value.get<Vector2i>() != Vector2i();
		else if (value.is<Vector2u>())
			return value.get<Vector2u>() != Vector2u();
		else if (value.is<Vector2f>())
			return value.get<Vector2f>() != Vector2f();
		else if (value.is<Vector3i>())
			return value.get<Vector3i>() != Vector3i();
		else if (value.is<Vector3u>())
			return value.get<Vector3u>() != Vector3u();
		else if (value.is<Vector3f>())
			return value.get<Vector3f>() != Vector3f();
		else if (value.is<Vector4i>())
			return value.get<Vector4i>() != Vector4i();
		else if (value.is<Vector4u>())
			return value.get<Vector4u>() != Vector4u();
		else if (value.is<Vector4f>())
			return value.get<Vector4f>() != Vector4f();

		return {};
	}

	std::optional<int32_t> getInt(const ConstantValue& value)
	{
		if (value.is<int32_t>())
			return value.get<int32_t>();
		else if (value.is<uint32_t>())
			return static_cast<int32_t>(value.get<uint32_t>());
		else if (value.is<float>())
			return static_cast<int32_t>(value.get<float>());

		return {};
	}

	std::optional<uint32_t> getUInt(const ConstantValue& value)
	{
		if (value.is<int32_t>())
			return static_cast<uint32_t>(value.get<int32_t>());
		else if (value.is<uint32_t>())
			return value.get<uint32_t>();
		else if (value.is<float>())
			return static_cast<uint32_t>(value.get<float>());

		return {};
	}

	std::optional<float> getFloat(const ConstantValue& value)
	{
		if (value.is<int32_t>())
			return static_cast<float>(value.get<int32_t>());
		else if (value.is<uint32_t>())
			return static_cast<float>(value.get<uint32_t>());
		else if (value.is<float>())
			return value.get<float>();

		return {};
	}

	template <size_t N, typename T>
	std::optional<Vector<N, T>> getVector(const ConstantValue& value)
	{
		if (value.is<int32_t>())
			return Vector<N, T>(static_cast<T>(value.get<int32_t>()));
		else if (value.is<uint32_t>())
			return Vector<N, T>(static_cast<T>(value.get<uint32_t>()));
		else if (value.is<float>())
			return Vector<N, T>(static_cast<T>(value.get<float>()));
		else if (value.is<Vector<N, int>>())
		{
			const auto& vec = value.get<Vector<N, int>>();

			Vector<N, T> result;

			for (size_t i = 0; i < N; i++)
				result[i] = static_cast<T>(vec[i]);

			return result;
		}
		else if (value.is<Vector<N, unsigned>>())
		{
			const auto& vec = value.get<Vector<N, unsigned>>();

			Vector<N, T> result;

			for (size_t i = 0; i < N; i++)
				result[i] = static_cast<T>(vec[i]);

			return result;
		}
		else if (value.is<Vector<N, float>>())
		{
			const auto& vec = value.get<Vector<N, float>>();

			Vector<N, T> result;

			for (size_t i = 0; i < N; i++)
				result[i] = static_cast<T>(vec[i]);

			return result;
		}

		return {};
	}

	template <typename T>
	std::optional<ConstantValue> scalarBinary(BinaryOperator op, T a, T b)
	{
		switch (op)
		{
			case BinaryOperator::Add: return a + b;
			case BinaryOperator::Subtract: return a - b;
			case BinaryOperator::Multiply: return a * b;
			case BinaryOperator::Divide: return a / b;
			case BinaryOperator::Power: return Math::pow(a, b);
			case BinaryOperator::Modulo: return Math::mod(a, b);
			case BinaryOperator::Less: return a < b;
			case BinaryOperator::Greater: return a > b;
			case BinaryOperator::Equal: return a == b;
			case BinaryOperator::NotEqual: return a != b;
			case BinaryOperator::LessOrEqual: return a <= b;
			case BinaryOperator::GreaterOrEqual: return a >= b;
			default: break;
		}

		return {};
	}

	template <size_t N, typename T>
	std::optional<ConstantValue> vectorBinary(BinaryOperator op, const Vector<N, T>& a, const Vector<N, T>& b)
	{
		switch (op)
		{
			case BinaryOperator::Add: return a + b;
			case BinaryOperator::Subtract: return a - b;
			case BinaryOperator::Multiply: return a * b;
			case BinaryOperator::Divide: return a / b;
			case BinaryOperator::Equal: return a == b;
			case BinaryOperator::NotEqual: return a != b;
			default: break;
		}

		return {};
	}

	template <size_t N, typename T>
	std::optional<ConstantValue> vectorScalarBinary(BinaryOperator op, const Vector<N, T>& a, T b)
	{
		switch (op)
		{
			case BinaryOperator::Add: return a + b;
			case BinaryOperator::Subtract: return a - b;
			case BinaryOperator::Multiply: return a * b;
			case BinaryOperator::Divide: return a / b;
			default: break;
		}

		return {};
	}
}

AstEvaluator::AstEvaluator()
{
}

AstEvaluator::~AstEvaluator()
{
}

std::optional<ConstantValue> AstEvaluator::evaluate(Expression& expression)
{
	expression.accept(*this);

	if (m_resultStack.empty())
		return {};

	auto value = m_resultStack.top();

	m_resultStack.pop();

	return value;
}

std::optional<bool> AstEvaluator::evaluateCondition(Expression& expression)
{
	auto value = evaluate(expression);

	if (!value)
		return std::nullopt;

	return getBool(value.value());
}

void AstEvaluator::visit(ConstantExpression& expression)
{
	m_resultStack.emplace(expression.value);
}

void AstEvaluator::visit(VariableExpression& expression)
{
	stackInvalidResult();
}

void AstEvaluator::visit(AccessIndexExpression& expression)
{
	stackInvalidResult();
}

void AstEvaluator::visit(AccessIdentifierExpression& expression)
{
	stackInvalidResult();
}

void AstEvaluator::visit(AssignmentExpression& expression)
{
	stackInvalidResult();
}

void AstEvaluator::visit(UnaryExpression& expression)
{
	auto& optionalResult = m_resultStack.emplace();

	auto optionalValue = evaluate(*expression.operand);

	if (!optionalValue)
		return;

	auto& value = optionalValue.value();

	switch (expression.op)
	{
		case UnaryOperator::IncrementPrefix:
		{
			if (value.is<int32_t>())
				optionalResult = ++value.get<int32_t>();
			else if (value.is<uint32_t>())
				optionalResult = ++value.get<uint32_t>();
			else if (value.is<float>())
				optionalResult = ++value.get<float>();
			else if (value.is<Vector2i>())
				optionalResult = ++value.get<Vector2i>();
			else if (value.is<Vector2u>())
				optionalResult = ++value.get<Vector2u>();
			else if (value.is<Vector2f>())
				optionalResult = ++value.get<Vector2f>();
			else if (value.is<Vector3i>())
				optionalResult = ++value.get<Vector3i>();
			else if (value.is<Vector3u>())
				optionalResult = ++value.get<Vector3u>();
			else if (value.is<Vector3f>())
				optionalResult = ++value.get<Vector3f>();
			else if (value.is<Vector4i>())
				optionalResult = ++value.get<Vector4i>();
			else if (value.is<Vector4u>())
				optionalResult = ++value.get<Vector4u>();
			else if (value.is<Vector4f>())
				optionalResult = ++value.get<Vector4f>();

			break;
		}
		case UnaryOperator::DecrementPrefix:
		{
			if (value.is<int32_t>())
				optionalResult = --value.get<int32_t>();
			else if (value.is<uint32_t>())
				optionalResult = --value.get<uint32_t>();
			else if (value.is<float>())
				optionalResult = --value.get<float>();
			else if (value.is<Vector2i>())
				optionalResult = --value.get<Vector2i>();
			else if (value.is<Vector2u>())
				optionalResult = --value.get<Vector2u>();
			else if (value.is<Vector2f>())
				optionalResult = --value.get<Vector2f>();
			else if (value.is<Vector3i>())
				optionalResult = --value.get<Vector3i>();
			else if (value.is<Vector3u>())
				optionalResult = --value.get<Vector3u>();
			else if (value.is<Vector3f>())
				optionalResult = --value.get<Vector3f>();
			else if (value.is<Vector4i>())
				optionalResult = --value.get<Vector4i>();
			else if (value.is<Vector4u>())
				optionalResult = --value.get<Vector4u>();
			else if (value.is<Vector4f>())
				optionalResult = --value.get<Vector4f>();

			break;
		}
		case UnaryOperator::IncrementPostfix:
		case UnaryOperator::DecrementPostfix:
		{
			if (value.is<int32_t>())
				optionalResult = value.get<int32_t>();
			else if (value.is<uint32_t>())
				optionalResult = value.get<uint32_t>();
			else if (value.is<float>())
				optionalResult = value.get<float>();
			else if (value.is<Vector2i>())
				optionalResult = value.get<Vector2i>();
			else if (value.is<Vector2u>())
				optionalResult = value.get<Vector2u>();
			else if (value.is<Vector2f>())
				optionalResult = value.get<Vector2f>();
			else if (value.is<Vector3i>())
				optionalResult = value.get<Vector3i>();
			else if (value.is<Vector3u>())
				optionalResult = value.get<Vector3u>();
			else if (value.is<Vector3f>())
				optionalResult = value.get<Vector3f>();
			else if (value.is<Vector4i>())
				optionalResult = value.get<Vector4i>();
			else if (value.is<Vector4u>())
				optionalResult = value.get<Vector4u>();
			else if (value.is<Vector4f>())
				optionalResult = value.get<Vector4f>();

			break;
		}
		case UnaryOperator::Positive:
		{
			optionalResult = value;

			break;
		}
		case UnaryOperator::Negative:
		{
			if (value.is<int32_t>())
				optionalResult = -value.get<int32_t>();
			else if (value.is<uint32_t>())
				optionalResult = -static_cast<int32_t>(value.get<uint32_t>());
			else if (value.is<float>())
				optionalResult = -value.get<float>();
			else if (value.is<Vector2i>())
				optionalResult = -value.get<Vector2i>();
			else if (value.is<Vector2u>())
			{
				const auto& v = value.get<Vector2u>();

				optionalResult = Vector2i(-v.x, -v.y);
			}
			else if (value.is<Vector2f>())
				optionalResult = -value.get<Vector2f>();
			else if (value.is<Vector3i>())
			{
				const auto& v = value.get<Vector3u>();

				optionalResult = Vector3i(-v.x, -v.y, -v.z);
			}
			else if (value.is<Vector3u>())
				optionalResult = -value.get<Vector3u>();
			else if (value.is<Vector3f>())
				optionalResult = -value.get<Vector3f>();
			else if (value.is<Vector4i>())
				optionalResult = -value.get<Vector4i>();
			else if (value.is<Vector4u>())
			{
				const auto& v = value.get<Vector4u>();

				optionalResult = Vector4i(-v.x, -v.y, -v.z, -v.w);
			}
			else if (value.is<Vector4f>())
				optionalResult = -value.get<Vector4f>();

			break;
		}
		case UnaryOperator::LogicalNot:
		{
			if (value.is<bool>())
				optionalResult = !value.get<bool>();

			break;
		}
		default:
			ATEMA_ERROR("Invalid unary operator");
	}
}

void AstEvaluator::visit(BinaryExpression& expression)
{
	auto& optionalResult = m_resultStack.emplace();

	const auto leftOptional = evaluate(*expression.left);
	const auto rightOptional = evaluate(*expression.right);

	if (!leftOptional || !rightOptional)
		return;

	auto left = leftOptional.value();
	auto right = rightOptional.value();

	bool leftBool = false;
	bool leftScalar = false;
	bool leftVector = false;

	if (left.is<bool>())
		leftBool = true;
	else if (left.isOneOf<int32_t, uint32_t, float>())
		leftScalar = true;
	else
		leftVector = true;

	bool rightBool = false;
	bool rightScalar = false;
	bool rightVector = false;

	if (right.is<bool>())
		rightBool = true;
	else if (right.isOneOf<int32_t, uint32_t, float>())
		rightScalar = true;
	else
		rightVector = true;

	if (leftBool && rightBool)
	{
		const bool l = left.get<bool>();
		const bool r = right.get<bool>();

		switch (expression.op)
		{
			case BinaryOperator::And:
			{
				optionalResult = l && r;
				break;
			}
			case BinaryOperator::Or:
			{
				optionalResult = l || r;
				break;
			}
			case BinaryOperator::Equal:
			{
				optionalResult = l == r;
				break;
			}
			case BinaryOperator::NotEqual:
			{
				optionalResult = l != r;
				break;
			}
			default:
				break;
		}
	}
	else if (leftScalar && rightScalar)
	{
		if (left.is<float>() || right.is<float>())
		{
			const auto l = getFloat(left);
			const auto r = getFloat(right);

			if (l && r)
				optionalResult = scalarBinary(expression.op, l.value(), r.value());
		}
		else if (left.is<int32_t>() || right.is<int32_t>())
		{
			const auto l = getInt(left);
			const auto r = getInt(right);

			if (l && r)
				optionalResult = scalarBinary(expression.op, l.value(), r.value());
		}
		else
		{
			const auto l = getUInt(left);
			const auto r = getUInt(right);

			if (l && r)
				optionalResult = scalarBinary(expression.op, l.value(), r.value());
		}
	}
	else if (leftVector && rightVector)
	{
		if (left.isOneOf<Vector2i, Vector2u, Vector2f>() && right.isOneOf<Vector2i, Vector2u, Vector2f>())
		{
			if (left.is<Vector2f>() || right.is<Vector2f>())
			{
				const auto l = getVector<2, float>(left);
				const auto r = getVector<2, float>(right);

				if (l && r)
					optionalResult = vectorBinary(expression.op, l.value(), r.value());
			}
			else if (left.is<Vector2i>() || right.is<Vector2i>())
			{
				const auto l = getVector<2, int>(left);
				const auto r = getVector<2, int>(right);

				if (l && r)
					optionalResult = vectorBinary(expression.op, l.value(), r.value());
			}
			else
			{
				const auto l = getVector<2, unsigned>(left);
				const auto r = getVector<2, unsigned>(right);

				if (l && r)
					optionalResult = vectorBinary(expression.op, l.value(), r.value());
			}
		}
		else if (left.isOneOf<Vector3i, Vector3u, Vector3f>() && right.isOneOf<Vector3i, Vector3u, Vector3f>())
		{
			if (left.is<Vector3f>() || right.is<Vector3f>())
			{
				const auto l = getVector<3, float>(left);
				const auto r = getVector<3, float>(right);

				if (l && r)
					optionalResult = vectorBinary(expression.op, l.value(), r.value());
			}
			else if (left.is<Vector3i>() || right.is<Vector3i>())
			{
				const auto l = getVector<3, int>(left);
				const auto r = getVector<3, int>(right);

				if (l && r)
					optionalResult = vectorBinary(expression.op, l.value(), r.value());
			}
			else
			{
				const auto l = getVector<3, unsigned>(left);
				const auto r = getVector<3, unsigned>(right);

				if (l && r)
					optionalResult = vectorBinary(expression.op, l.value(), r.value());
			}
		}
		else if (left.isOneOf<Vector4i, Vector4u, Vector4f>() && right.isOneOf<Vector4i, Vector4u, Vector4f>())
		{
			if (left.is<Vector4f>() || right.is<Vector4f>())
			{
				const auto l = getVector<4, float>(left);
				const auto r = getVector<4, float>(right);

				if (l && r)
					optionalResult = vectorBinary(expression.op, l.value(), r.value());
			}
			else if (left.is<Vector4i>() || right.is<Vector4i>())
			{
				const auto l = getVector<4, int>(left);
				const auto r = getVector<4, int>(right);

				if (l && r)
					optionalResult = vectorBinary(expression.op, l.value(), r.value());
			}
			else
			{
				const auto l = getVector<4, unsigned>(left);
				const auto r = getVector<4, unsigned>(right);

				if (l && r)
					optionalResult = vectorBinary(expression.op, l.value(), r.value());
			}
		}
	}
	else if ((leftScalar && rightVector) || (leftVector && rightScalar))
	{
		// We will take left as the vector and right as the scalar in any case (same result)
		if (leftScalar)
			std::swap(left, right);

		if (left.isOneOf<Vector2i, Vector2u, Vector2f>())
		{
			if (left.is<Vector2f>() || right.is<float>())
			{
				const auto l = getVector<2, float>(left);
				const auto r = getFloat(right);

				if (l && r)
					optionalResult = vectorScalarBinary(expression.op, l.value(), r.value());
			}
			else if (left.is<Vector2i>() || right.is<int32_t>())
			{
				const auto l = getVector<2, int>(left);
				const auto r = getInt(right);

				if (l && r)
					optionalResult = vectorScalarBinary(expression.op, l.value(), r.value());
			}
			else if (left.is<Vector2u>() || right.is<uint32_t>())
			{
				const auto l = getVector<2, unsigned>(left);
				const auto r = getUInt(right);

				if (l && r)
					optionalResult = vectorScalarBinary(expression.op, l.value(), r.value());
			}
		}
		else if (left.isOneOf<Vector3i, Vector3u, Vector3f>())
		{
			if (left.is<Vector3f>() || right.is<float>())
			{
				const auto l = getVector<3, float>(left);
				const auto r = getFloat(right);

				if (l && r)
					optionalResult = vectorScalarBinary(expression.op, l.value(), r.value());
			}
			else if (left.is<Vector3i>() || right.is<int32_t>())
			{
				const auto l = getVector<3, int>(left);
				const auto r = getInt(right);

				if (l && r)
					optionalResult = vectorScalarBinary(expression.op, l.value(), r.value());
			}
			else if (left.is<Vector2u>() || right.is<uint32_t>())
			{
				const auto l = getVector<3, unsigned>(left);
				const auto r = getUInt(right);

				if (l && r)
					optionalResult = vectorScalarBinary(expression.op, l.value(), r.value());
			}
		}
		else if (left.isOneOf<Vector4i, Vector4u, Vector4f>())
		{
			if (left.is<Vector4f>() || right.is<float>())
			{
				const auto l = getVector<4, float>(left);
				const auto r = getFloat(right);

				if (l && r)
					optionalResult = vectorScalarBinary(expression.op, l.value(), r.value());
			}
			else if (left.is<Vector4i>() || right.is<int32_t>())
			{
				const auto l = getVector<4, int>(left);
				const auto r = getInt(right);

				if (l && r)
					optionalResult = vectorScalarBinary(expression.op, l.value(), r.value());
			}
			else if (left.is<Vector2u>() || right.is<uint32_t>())
			{
				const auto l = getVector<4, unsigned>(left);
				const auto r = getUInt(right);

				if (l && r)
					optionalResult = vectorScalarBinary(expression.op, l.value(), r.value());
			}
		}
	}
}

void AstEvaluator::visit(FunctionCallExpression& expression)
{
	stackInvalidResult();
}

void AstEvaluator::visit(BuiltInFunctionCallExpression& expression)
{
	stackInvalidResult();
}

void AstEvaluator::visit(CastExpression& expression)
{
	stackInvalidResult();
	return;

	auto& optionalResult = m_resultStack.emplace();

	std::vector<ConstantValue> components;

	for (auto& component : expression.components)
	{
		auto constant = evaluate(*component);

		if (!constant)
			return;

		components.emplace_back(constant.value());
	}

	if (expression.type.is<PrimitiveType>() && components.size() == 1)
	{
		optionalResult = components[0];
	}
	else if (expression.type.is<VectorType>())
	{
		const auto& type = expression.type.get<VectorType>();

		if (type.componentCount != components.size())
			return;

		std::optional<ConstantValue> optionalValue;

		switch (type.primitiveType)
		{
			case PrimitiveType::Int:
			{
				int* data = nullptr;

				if (type.componentCount == 2)
				{
					optionalValue = Vector2i();
					data = &(optionalValue.value().get<Vector2i>().data[0]);
				}
				else if (type.componentCount == 3)
				{
					optionalValue = Vector3i();
					data = &(optionalValue.value().get<Vector3i>().data[0]);
				}
				else if (type.componentCount == 4)
				{
					optionalValue = Vector4i();
					data = &(optionalValue.value().get<Vector4i>().data[0]);
				}
				else
				{
					return;
				}

				for (size_t i = 0; i < components.size(); i++)
				{
					const auto value = getInt(components[i]);

					if (!value)
						return;

					data[i] = static_cast<int>(value.value());
				}

				break;
			}
			case PrimitiveType::UInt:
			{
				unsigned* data = nullptr;

				if (type.componentCount == 2)
				{
					optionalValue = Vector2u();
					data = &(optionalValue.value().get<Vector2u>().data[0]);
				}
				else if (type.componentCount == 3)
				{
					optionalValue = Vector3u();
					data = &(optionalValue.value().get<Vector3u>().data[0]);
				}
				else if (type.componentCount == 4)
				{
					optionalValue = Vector4u();
					data = &(optionalValue.value().get<Vector4u>().data[0]);
				}
				else
				{
					return;
				}

				for (size_t i = 0; i < components.size(); i++)
				{
					const auto value = getUInt(components[i]);

					if (!value)
						return;

					data[i] = static_cast<unsigned>(value.value());
				}

				break;
			}
			case PrimitiveType::Float:
			{
				float* data = nullptr;

				if (type.componentCount == 2)
				{
					optionalValue = Vector2f();
					data = &(optionalValue.value().get<Vector2f>().data[0]);
				}
				else if (type.componentCount == 3)
				{
					optionalValue = Vector3f();
					data = &(optionalValue.value().get<Vector3f>().data[0]);
				}
				else if (type.componentCount == 4)
				{
					optionalValue = Vector4f();
					data = &(optionalValue.value().get<Vector4f>().data[0]);
				}
				else
				{
					return;
				}

				for (size_t i = 0; i < components.size(); i++)
				{
					const auto value = getFloat(components[i]);

					if (!value)
						return;

					data[i] = value.value();
				}

				break;
			}
			default:
				break;
		}

		optionalResult = std::move(optionalValue);
	}
}

void AstEvaluator::visit(SwizzleExpression& expression)
{
	stackInvalidResult();
}

void AstEvaluator::visit(TernaryExpression& expression)
{
	auto& optionalResult = m_resultStack.emplace();

	const auto condition = evaluate(*expression.condition);

	if (!condition.has_value() || !getBool(condition.value()))
		return;

	const auto optionalBool = getBool(condition.value());

	if (!optionalBool.has_value())
		return;

	if (optionalBool.value())
		optionalResult = evaluate(*expression.trueValue);
	else
		optionalResult = evaluate(*expression.falseValue);
}

void AstEvaluator::stackInvalidResult()
{
	m_resultStack.emplace();
}
