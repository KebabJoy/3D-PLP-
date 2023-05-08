require 'json'

f = JSON.parse File.read('./experiment_data/4514-97_9.json')
result = f["boxes"].map do |box|
    [
    box["position"]["z"] - (box["calculated_size"]["width"] / 2),
     box["position"]["x"] - (box["calculated_size"]["length"] / 2),
      box["position"]["y"] - (box["calculated_size"]["height"] / 2),
        box["calculated_size"]["width"],
         box["calculated_size"]["length"],
          box["calculated_size"]["height"]
          ]
end

total_intersections = 0
total_perimeter = 0
result.size.times do |res_ind|
    if result[res_ind][2] != 0
        total_perimeter += 2 * (result[res_ind][3] + result[res_ind][4]);
    else
        total_perimeter += (result[res_ind][3] + result[res_ind][4]);
    end

    (res_ind+1...result.size).each do |box_ind|
        next if result[res_ind][0] != result[box_ind][0] || result[res_ind][1] != result[box_ind][1]
        next if result[res_ind][2] + result[res_ind][5] != result[box_ind][2] && result[box_ind][2] + result[box_ind][5] != result[res_ind][2]
        matching_length = 0

        2.times do |k|
          4.times do |l|
            edge1 = [result[res_ind][k], result[res_ind][(k+1)%3], result[res_ind][(k+2)%3]]
            edge2 = [result[box_ind][k], result[box_ind][(k+1)%3], result[box_ind][(k+2)%3]]
            if edge1[0] == edge2[0] && edge1[1] == edge2[1]
              matching_length += [result[res_ind][k+3], result[box_ind][k+3]].min
            end
          end
        end

        total_intersections += matching_length
    end
end

pp total_intersections / total_perimeter