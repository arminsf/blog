import json
from itertools import pairwise
from math import radians

with open("third_party/ne_110m_land.geojson", 'r') as f:
    data = json.load(f)

lines = []

for feature in data["features"]:
    if feature["geometry"]["type"] == "Polygon":
        for shape in feature["geometry"]["coordinates"]:
            lines += list(pairwise(map(tuple, shape)))
    if feature["geometry"]["type"] == "MultiPolygon":
        for polygon in feature["geometry"]["coordinates"]:
            for shape in polygon:
                lines += list(pairwise(map(tuple, shape)))

print("constexpr std::vector<demo::Line2> coordinate_data = {")

for line in lines:
    print("\t{{", radians(line[0][0]), "f,", radians(line[0][1]), "f}, {", radians(line[1][0]), "f,", radians(line[1][1]), "f}},", sep="")

print("};")