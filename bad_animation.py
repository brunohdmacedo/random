# It plots speed distribution frame by frame
# I know that it seems like bad code. I tried to use matplotlib.animation, but I failed.
import numpy as np
import matplotlib.pyplot as plt
from particles_system import ParticlesSystem

BINS = 100
SPEED_LIM = 4000
Y_LIM = 0.003
DENSITY = True
dt = 1e-13

system = ParticlesSystem(radius=2e-10, mass=3.3e-26, volume=1e-23, pressure=10e5, temperature=300, init_type='uniform')

fig = plt.figure('Maxwell speed distribution', figsize=(16, 12), dpi=200, facecolor='w')
ax = [[0, 0], [0, 0]]
for i in range(2):
    for j in range(2):
        ax[i][j] = fig.add_subplot(2, 2, i * 2 + j + 1)
fig.subplots_adjust(wspace=0.4, hspace=0.4)

# Theoretical Maxwell distribution
v_mag = np.linspace(0, SPEED_LIM, 1000)
v_proj = np.linspace(-SPEED_LIM / 2, SPEED_LIM / 2, 1000)
prob_proj = ((system.mass / (2 * np.pi * 1.38062e-23 * system.temperature)) ** (1 / 2)) * np.exp(
    -system.mass * np.square(v_proj) / (2 * 1.38062e-23 * system.temperature))
prob_mag = 4 * np.pi * np.square(v_mag) * (
        (system.mass / (2 * np.pi * 1.38062e-23 * system.temperature)) ** (3 / 2)) * np.exp(
    -system.mass * np.square(v_mag) / (2 * 1.38062e-23 * system.temperature))


def plot_speed_dist(s: ParticlesSystem, savepath):
    for i in range(2):
        for j in range(2):
            ax[i][j].clear()
    ax[0][0].set_xlabel('speed x proj [m/sec]')
    ax[0][0].set_xlim(-SPEED_LIM / 2, SPEED_LIM / 2)
    ax[0][0].set_ylabel('probability')
    ax[0][0].set_ylim(0, Y_LIM)
    ax[0][0].set_label('distribution v_x')
    ax[0][0].plot(v_proj, prob_proj, color='red')
    ax[0][0].hist(s.get_velocities_x(), bins=BINS, range=(-SPEED_LIM / 2, SPEED_LIM / 2), density=DENSITY)

    ax[0][1].set_xlabel('speed y proj [m/sec]')
    ax[0][1].set_xlim(-SPEED_LIM / 2, SPEED_LIM / 2)
    ax[0][1].set_ylabel('probability')
    ax[0][1].set_ylim(0, Y_LIM)
    ax[0][1].set_label('distribution v_y')
    ax[0][1].plot(v_proj, prob_proj, color='red')
    ax[0][1].hist(s.get_velocities_y(), bins=BINS, range=(-SPEED_LIM / 2, SPEED_LIM / 2), density=DENSITY)

    ax[1][0].set_xlabel('speed z proj [m/sec]')
    ax[1][0].set_xlim(-SPEED_LIM / 2, SPEED_LIM / 2)
    ax[1][0].set_ylabel('probability')
    ax[1][0].set_ylim(0, Y_LIM)
    ax[1][0].set_label('distribution v_z')
    ax[1][0].plot(v_proj, prob_proj, color='red')
    ax[1][0].hist(s.get_velocities_z(), bins=BINS, range=(-SPEED_LIM / 2, SPEED_LIM / 2), density=DENSITY)

    ax[1][1].set_xlabel('speed magnitude [m/sec]')
    ax[1][1].set_xlim(0, SPEED_LIM)
    ax[1][1].set_ylabel('probability')
    ax[1][1].set_ylim(0, Y_LIM)
    ax[1][1].set_label('distribution v_mag')
    ax[1][1].plot(v_mag, prob_mag, color='red')
    ax[1][1].hist(s.get_velocities_magnitude(), bins=BINS, range=(0, SPEED_LIM), density=DENSITY)
    fig.savefig(savepath)
    # plt.show()


counter = 0

while 1:
    if counter % 10 == 0:
        plot_speed_dist(system, f'data/1/{counter}.png')
        print('sved figure ', counter)
    system.iteration(dt=dt)
    counter += 1
