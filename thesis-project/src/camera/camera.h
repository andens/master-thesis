#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <DirectXMath.h>
#include <functional>

class Camera
{
public:
	Camera( void );
	~Camera( void );

	// Get/Set world camera position.
	DirectX::XMVECTOR GetPositionXM( void ) const;
	DirectX::XMFLOAT3 GetPosition( void ) const;
	void SetPosition( float x, float y, float z );
	void SetPosition( const DirectX::XMFLOAT3& v );

	// Get camera basis vectors.
	DirectX::XMVECTOR GetRightXM( void ) const;
	DirectX::XMFLOAT3 GetRight( void ) const;
	DirectX::XMVECTOR GetUpXM( void ) const;
	DirectX::XMFLOAT3 GetUp( void ) const;
	DirectX::XMVECTOR GetLookXM( void ) const;
	DirectX::XMFLOAT3 GetLook( void ) const;

	// Get frustum properties.
	float GetNearZ( void ) const;
	float GetFarZ( void ) const;
	float GetAspect( void) const;
	float GetFovY( void ) const;
	float GetFovX( void ) const;

	// Get near and far plane dimensions in view space coordinates.
	float GetNearWindowWidth( void ) const;
	float GetNearWindowHeight( void ) const;
	float GetFarWindowWidth( void ) const;
	float GetFarWindowHeight( void ) const;

	// Set frustum
	void SetLens( float fovY, float aspect, float zn, float zf );

	// Define camera space via LookAt parameters.
	void LookAt( DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp );
	void LookAt( const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up );

	// Get View/Proj matrices.
	DirectX::XMMATRIX View( void ) const;
	DirectX::XMMATRIX Proj( void ) const;
	DirectX::XMMATRIX ViewProj( void ) const;

	// Strafe/Walk the camera a distance d.
	void Strafe( float d );
	void Walk( float d );

	// Rotate the camera.
	void Pitch( float angle );
	void RotateY( float angle );

	// After modifying the camera position/orientation, call to rebuild the view
	// matrix once per frame.
	void UpdateViewMatrix( void );

	// TODO: Better event system
	void SetViewChangedListener( std::function<void( const Camera &camera )> listener ) { mViewListener = listener; }
	void SetProjChangedListener( std::function<void( const Camera &camera )> listener ) { mProjListener = listener; }

private:
	// Camera coordinate system with coordinates relative to world space.
	DirectX::XMFLOAT3 mPosition; // View space origin
	DirectX::XMFLOAT3 mRight;	 // View space x-axis
	DirectX::XMFLOAT3 mUp;		 // View space y-axis
	DirectX::XMFLOAT3 mLook;	 // View space z-axis

	// Cache frustum properties
	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;
	float mNearWindowHeight;
	float mFarWindowHeight;

	// Cache View/Proj matrices.
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;

	// TODO: better event
	std::function<void( const Camera &camera )> mViewListener;
	std::function<void( const Camera &camera )> mProjListener;
};

#endif // _CAMERA_H_