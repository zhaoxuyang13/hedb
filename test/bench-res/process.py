import csv
import os
filePath = "/home/xuyang/Codes/fvp-demo/hedb/benchmark/results/"
fileNameTemp = "output.{number}.res"

def integers(a, b):
    return [i for i in range(a, b+1)]


test_cases = {}
for i in range(22):
    base = 87 + i
    test_cases[i] = [base]
# test_cases = {
#     "1": integers(63,69),
#     "2": integers(70,78),
# }
def write_csv_file(writer, data):
    writer.writerow(data)
def write_empty_line(writer):
    writer.writerow("")
def read_csv_file_data(name):
    with open(name, "r") as f: 
        title = f.readline().split(",")
        data = f.readline().split(",")
        zipped = zip(title,data)
        return title,data
def process_csv_files(out_filename,file_lists):
    with open(out_filename, 'w', newline='') as file:
        writer = csv.writer(file)
        for case_num, file_list in file_lists.items(): 
            write_csv_file(writer,["terminal=" + str(case_num)] )
            title_write = False
            for filename in file_list:
                title,data = read_csv_file_data(filePath + filename)
                if not title_write:
                    write_csv_file(writer, title)
                    title_write = True
                write_csv_file(writer, data)
            writer.writerow("")

def convert_filename(case_lists):
    for index,case_list in case_lists.items():
        case_list = [fileNameTemp.format(number = case) for case in case_list]
        case_lists[index] = case_list
  
file_lists = convert_filename(test_cases)

process_csv_files("test1.csv", test_cases)


# with open('out-2.csv', 'w', newline='') as file:
#     writer = csv.writer(file)
#     title_write = False
#     for outNum in outs:
#         if outNum == 0:
#             fileName = "output.res"
#         else :
#             fileName = fileNameTemp.format(number = outNum)
#         with open(filePath + fileName, "r") as f:
#             title = f.readline().split(",")
#             if not title_write:
#                 writer.writerow(title)
#                 title_write = True
                
#             data = f.readline().split(",");
#             zipped = zip(title,data)
#             # print(zipped)
#             writer.writerow(data)
