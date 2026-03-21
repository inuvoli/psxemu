# filter_lines.py

KEYWORDS_INCLUDE = [
    #"INT - Request Hardware Interrupt",
    #"INT - Cancel Hardware Interrupt",
    #"CPU - Throw Exception",
    #"COP0 - Executed RFE",
    #"CP0 - mfc0",
    #"CP0 - mtc0",
    #"KRN - Calling",
    #"INT - Write I_STAT Register",
    #"INT - Read I_STAT Register",
    #"INT - Write I_MASK Register",
    #"INT - Read I_MASK Register"
    #"CDR - Read RESULT Fifo",
    #"CPU - Exception",
    #"INT - ",
    #"CTR - ",
    ""
]

KEYWORDS_EXCLUDE = [
    "RND - Set Semi Transparency to: false",
    "GPU - Monocrome Rectangle, 1x1, opaque (params: 1)",
    "GPU - Reset Command Buffer (params: 0)",
    "INT - Hardware Interrupt Request (vBlank)",
    "RND - vBlank! vBlank! vBlank! vBlank! vBlank! vBlank! vBlank!"

]

def filter_file(input_path, output_path):
    with open(input_path, "r", encoding="utf-8", errors="ignore") as infile, \
         open(output_path, "w", encoding="utf-8") as outfile:

        for line in infile:
            if any(k in line for k in KEYWORDS_INCLUDE) and not any(
                e in line for e in KEYWORDS_EXCLUDE
            ):
                outfile.write(line)

if __name__ == "__main__":
    input_file = "debug.log"
    output_file = "output.log"

    filter_file(input_file, output_file)
    print("Filtering complete.")
