require 'json'

f = JSON.parse File.read('./experiment_data/4514-97_9.json')
# a = f["first_visual"]["boxes"].group_by {|arr| arr["sort"]}
# res = a[a.keys.first].map do |box|
# res = f["first_visual"]["boxes"].filter { |box| box["margin"]["width"] > 0 || box["margin"]["length"] > 0 || box["margin"]["height"] > 0 }
res = f["boxes"].map do |box|
    [
    box["position"]["z"] - (box["calculated_size"]["width"] / 2),
     box["position"]["x"] - (box["calculated_size"]["length"] / 2),
      box["position"]["y"] - (box["calculated_size"]["height"] / 2),
        box["calculated_size"]["width"],
         box["calculated_size"]["length"],
          box["calculated_size"]["height"]
          ]
end
ans = [{result: res}]
File.open('pop_data.json', 'w') { |file| file.write(ans.to_json) }
# 9 8 5 1
res.each_with_index do |v, i|
    res.each_with_index do |v1, j|
        next if i == j
        if boxes_intersect?(v, v1)
            pp i
            pp j
        end
    end
end

pp 2

def parse
    i= 0
    res = f["boxes"].map do |box|
        i += 1
        {
            id: i,
            sizeX: box["calculated_size"]["width"],
             sizeY: box["calculated_size"]["length"],
              sizeZ: box["calculated_size"]["height"],
              lPerimeter: box["calculated_size"]["width"] * 4 + box["calculated_size"]["length"] * 4,
              value: rand(50..500)
         }
    end
    File.open('test-2.json', 'w') { |f| f.write(res.to_json) }
end

def boxes_intersect?(box1, box2)
  # Determine the minimum and maximum x, y, and z values for each box
  x1_min, y1_min, z1_min, _, _, _ = box1
  x1_max = x1_min + box1[3]
  y1_max = y1_min + box1[4]
  z1_max = z1_min + box1[5]

  x2_min, y2_min, z2_min, _, _, _ = box2
  x2_max = x2_min + box2[3]
  y2_max = y2_min + box2[4]
  z2_max = z2_min + box2[5]

  # Check if the boxes overlap in any dimension
  x_overlap = (x1_min < x2_max) && (x1_max > x2_min)
  y_overlap = (y1_min < y2_max) && (y1_max > y2_min)
  z_overlap = (z1_min < z2_max) && (z1_max > z2_min)

  # Check if the boxes share any common points along each dimension
  x_common_points = (x1_min == x2_max) || (x1_max == x2_min)
  y_common_points = (y1_min == y2_max) || (y1_max == y2_min)
  z_common_points = (z1_min == z2_max) || (z1_max == z2_min)

  # If the boxes overlap in all three dimensions and don't just contact at their borders, they intersect
  return x_overlap && y_overlap && z_overlap && !(x_common_points || y_common_points || z_common_points)
end

def boxes_intersections?(boxes)
  # Iterate through all pairs of boxes and check if they intersect
  for i in 0...(boxes.length - 1)
    for j in (i+1)...boxes.length
      if boxes_intersect?(boxes[i], boxes[j])
        pp i
        pp j
        return true
      end
    end
  end

  # If no intersecting pair was found, return false
  return false
end

