
object picking 이 된 물체에서 f키를 눌렀을 때, face normal vector가 켜지도록 하고, n을 눌렀을 때는 vertex normal vector가 켜지도록 합니다.
이는 obj 파일에서 face normal vector와 vertex normal vector를 직접 구해주었기 때문인데, face normal vector는 vertex를 이용해, 각각 vertex로 만든 두개의 변(벡터)를 외적시켜서 구해줍니다. vertex normal vector는 모든 vertex에 대해서, 모든 face 리스트들을 다 돌면서 그 face가 현재 찾고 있는 vertex를 가지고 있는지 확인합니다. 만약 가지고 있을 때는 그를 vertex normal vector에 더해주고, 그렇게 전부다 돈 후에 그 버텍스 주변에 있는 면의 개수로 나눠준 후 normalize해 줍니다.

광원을 키는 것은 gllightfv 함수를 통해서 구현합니다.
각각 light마다 pos의 마지막이 0.0일 경우는 infinite한 거리에 있는 광원, 1.0 에 있는 경우는 finite한 거리에 있는 광원이고, 각각을 ambient, diffuse, specular를 이용해서 표현하였습니다. 색깔도 각각 다르게 하였고, 해당하는 위치에 구를 두었습니다.