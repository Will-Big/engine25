#pragma once
#include "Common.hpp"

namespace core
{
    struct Transform
	{
        vec3 position = vec3(0.0f);
        quat rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
        vec3 scale = vec3(1.0f);

        // 월드 행렬 생성
        mat4 ToMatrix() const;

        static mat4 ToMatrix(vec3 position, quat rotation, vec3 scale);

        // 행렬로부터 위치/회전/스케일 분해
        static Transform FromMatrix(mat4& matrix);
	};

    inline mat4 Transform::ToMatrix() const
    {
	    const mat4 t = DirectX::SimpleMath::Matrix::CreateTranslation(position);
	    const mat4 r = DirectX::SimpleMath::Matrix::CreateFromQuaternion(rotation);
	    const mat4 s = DirectX::SimpleMath::Matrix::CreateScale(scale);
	    return t * r * s;
    }

    inline mat4 Transform::ToMatrix(vec3 position, quat rotation, vec3 scale)
    {
	    const mat4 t = DirectX::SimpleMath::Matrix::CreateTranslation(position);
	    const mat4 r = DirectX::SimpleMath::Matrix::CreateFromQuaternion(rotation);
	    const mat4 s = DirectX::SimpleMath::Matrix::CreateScale(scale);
	    return t * r * s;
    }

    inline Transform Transform::FromMatrix(mat4& matrix)
    {
	    vec3 scale;
	    vec3 translation;
	    quat rotation;

	    if (matrix.Decompose(scale, rotation, translation))
	    {
		    Transform transform;
		    transform.position = translation;
		    transform.rotation = rotation;
		    transform.scale = scale;
		    return transform;
	    }
	    else
	    {
		    // 변환 실패 시 기본값 반환
		    return {};
	    }
    }
}
