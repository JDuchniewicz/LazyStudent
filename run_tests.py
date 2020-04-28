import argparse
import os
import subprocess


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', help='name of file', required=True)
    parser.add_argument('-n', help='numer of times to run', required=True)
    args = parser.parse_args()

    if os.path.exists("../results.txt"):
        os.remove("../results.txt")

    # run the file n times
    for i in range(1, int(args.n) + 1):
        name = f"./test/{args.f}{i}"
        subprocess.run(["./build/LazyStudent_genetic", name])#, stdout=subprocess.PIPE)
         

if __name__ == "__main__":
    main()
