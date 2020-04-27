import argparse
import random
import math

class Generator:
    def __init__(self, args):
        self.semesterN = int(args.n)
        self.ECTSPerSem = int(args.e)
        self.maxDaysPerSem = int(args.m)
        self.subjectN = int(args.s)
        self.fileName = "test/" + args.d
        self.minECTSPerSubject = int(self.ECTSPerSem / int(self.subjectN / self.semesterN))
        self.maxDaysPerSubject = int(self.maxDaysPerSem / int(self.subjectN / self.semesterN))


    def generate_test(self):
        print("max days per sub: " + str(self.maxDaysPerSubject))
        print("min ects per sub: " + str(self.minECTSPerSubject))
        # generate subjectN dependency lists
        dep = [[] for i in range(self.subjectN)]
        # generate a cost List of N
        sub = []
        for i in range(1, self.subjectN + 1):
            # generate a dependency with some p
            p = random.randint(1, 100)
            if p > 90:
                where = random.randint(1, self.subjectN)
                dep[where].append(i)
            
            ects = random.randint(self.minECTSPerSubject, self.minECTSPerSubject * 2)
            days = random.randint(int(self.maxDaysPerSubject / 2), self.maxDaysPerSubject)
            sub.append((days, ects))

        
        # print the subjects to file
        out = open(self.fileName, 'w')
        line = f"{self.semesterN} {self.ECTSPerSem} {self.maxDaysPerSem}"
        print(line)
        out.write(line + '\n')

        for pair in sub:
            line = f"{pair[0]} {pair[1]}"
            print(line)
            out.write(line + '\n')
        
        out.write('#\n')
        for d in dep:
            if len(d) > 0:
                for i in d:
                    print("dep: " + str(i) + " -> " + str(dep.index(d)))
                    line = f"{i} {dep.index(d)}"
                    out.write(line + '\n')

                

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-n', help='number of semesters', required=True)
    parser.add_argument('-e', help='number of ECTS per semester', required=True)
    parser.add_argument('-m', help='maxium number of days to study per semester', required=True)
    parser.add_argument('-s', help='number of subjects', required=True)
    parser.add_argument('-d', help='output file name', required=True)
    args = parser.parse_args()
    gen = Generator(args)     
    gen.generate_test()

if __name__ == "__main__":
    main()

