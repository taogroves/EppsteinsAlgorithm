from solutions.pcSol_python import shortest_paths_no_same_arrival
import os
import random
# given an index of the test, generate the test case, run the function on it, and save input/output to ./io/test.in.[index] and ./io/test.out.[index]


test_2 = """3 3 0 2 1000
0 1 1
1 0 1
1 2 1"""

def generate_random_test(i, n, m, s, t, k):
    # Generate a random test case
    from io import StringIO

    # Create a random graph with n nodes and m edges
    edges = []
    for _ in range(m):
        u = random.randint(0, n - 1)
        v = random.randint(0, n - 1)
        w = random.randint(1, 1000)
        edges.append((u, v, w))

    print(f"Generated {n} nodes, {m} edges, s={s}, t={t}, k={k}")

    # Create the input string
    input_str = f"{n} {m} {s} {t} {k}\n"
    for u, v, w in edges:
        input_str += f"{u} {v} {w}\n"

    # Save the input and output to files
    with open(f'./io/test.in.{i}', 'w') as f:
        f.write(input_str)
    os.system(f'python3 ./solutions/pcSol_python.py < ./io/test.in.{i} > ./io/test.out.{i}')
    return input_str

def generate_test(index):
    # Generate the test case
    if index == 2:
        test = test_2
    else:
        raise ValueError("Test case not found")

    # Run the function on the test case
    input_data = test.split('\n')
    n, m, s, t, k = map(int, input_data[0].split())
    g = [[] for _ in range(n)]
    for i in range(m):
        u, v, w = map(int, input_data[i + 1].split())
        g[u].append((w, v))

    # Save the input and output to files
    with open(f'./io/test.in.{index}', 'w') as f:
        f.write(test)

    os.system(f'python3 ./solutions/pcSol_python.py < ./io/test.in.{index} > ./io/test.out.{index}')


def run_tests_in_range(start, end):
    # Run the test case
    for i in range(start, end + 1):
        print (f"Running test case {i}")
        os.system(f'python3 ./solutions/pcSol_python.py < ./io/test.in.{i} > ./io/test.out.{i}')

if __name__ == "__main__":
    # generate_test(2)

    # generate_random_test(11, 10, 1000000, 0, 9, 3)

    run_tests_in_range(14, 14)

    # for i in range(12, 21):
    #     generate_random_test(i, random.randint(10, 10000), random.randint(100, 100000), 0, 9, random.randint(1, 25))
    #     os.system(f'python3 ./solutions/pcSol_python.py < ./io/test.in.{i} > ./io/test.out.{i}')