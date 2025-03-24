![image](https://github.com/user-attachments/assets/a84273bd-84cc-4852-abef-0b1ba5418de5)
Main EmptyViewer.cpp

main() 함수에서 카메라(Camera) 및 장면(Scene) 을 초기화하고, 평면과 구체를 생성하여 Scene에 등록

렌더 함수(render())를 호출하여 각 픽셀에 대한 광선을 생성하고, 교차 여부에 따라 흑백으로 출력

GLFW 루프를 통해 결과 이미지를 계속 화면에 표시

Ray 클래스

광선의 원점(origin)과 방향(direction)을 저장하며, 방향을 정규화

Camera 클래스

카메라 위치(eye), 뷰잉 영역 파라미터(l, r, b, t, d)를 보관

픽셀 좌표(i, j)에 대응하는 광선(Ray)을 생성하는 메서드(generateRay) 제공

Surface 추상 클래스

intersect(const Ray&) 메서드를 순수 가상 함수로 선언하여, 자식 클래스(Plane, Sphere)에서 구현하도록 함

Plane 클래스

y = constant 형태의 평면과 광선의 교차를 계산

Sphere 클래스

구체의 중심, 반지름을 보관

광선과의 교차(2차 방정식) 해를 구해 t값 반환

Scene 클래스

여러 Surface 객체(Plane, Sphere 등)를 저장

주어진 광선에 대해 가장 가까운 교차를 찾는 findNearest 메서드 제공

4. 실행 결과
다음과 같이 구체 3개와 평면이 흑백으로 렌더링된 이미지를 확인할 수 있습니다.


흰색: 광선이 객체와 교차한 경우

검은색: 교차하지 않은 경우

5. 라이선스/출처
OpenGL/GLFW/GLM 등은 해당 라이브러리의 라이선스를 따릅니다.
