import json
import matplotlib.pyplot as plt


def plot_stats(average_fitness, title=""):
    x1 = range(len(average_fitness))
    avg_freespace = []
    avg_number = []
    avg_value = []

    for item in average_fitness:
        # avg_freespace.append(item[0])
        # avg_value.append(item[2])
        avg_freespace.append(item[0])
        avg_number.append(item[1])
        avg_value.append(item[3])

    # plt.plot(x1, avg_freespace, label='Average Occupied Volume')
    # plt.plot(x1, avg_freespace, label='Average Lsum')
    # plt.plot(x1, avg_number, label='Total Perimeter')
    plt.plot(x1, avg_value, label='Lsum / LSumPer')
    plt.xlabel('Number of Generations')
    plt.ylabel('Fitness Values')
    plt.xticks(ticks=[t for t in x1 if t % 20 == 0])
    plt.title(title)
    plt.legend()
    plt.show()


with open('avg_fitness.json') as f:
    data = json.load(f)

accuracy = [p[3] for p in data]
plot_stats(data)