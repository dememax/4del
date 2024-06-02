# chosing washing machine
# Maxim Dementyev, 2024

PRICE=1
LINK=2
TRADEMARK=3
DRUM=4 # Tambour
MODEL=6 # string
CAPACITY=7 # kg
HEIGHT=8 # cm
WIDTH=9 # cm
DEPTH=10 # cm
NOISE=11 # dB
SALES=13

GLOBAL_DICT= {
    PRICE: "Price",
    LINK: "Link",
    TRADEMARK: "Trademark",
    DRUM: "Drum",
    MODEL: "Model",
    CAPACITY: "Capacity",
    HEIGHT: "Height",
    WIDTH: "Width",
    DEPTH: "Depth",
    NOISE: "Noise",
}

PRODUCTS= {
    "Whirlpool": { # TRADEMARK
        "FFWDD1176258BCVFR": # MODEL
        {
            DRUM: 71,
            CAPACITY: 11,
            HEIGHT: 85,
            WIDTH: 59.5,
            DEPTH: 63.5,
            NOISE: 80,
            SALES: {
                "https://www.boulanger.com/ref/1178218": # LINK
                    699, # PRICE
            },
        },
        "FFDB10469BVFR": # TRADEMARK, MODEL
        {
            DRUM: 71,
            CAPACITY: 10,
            HEIGHT: 85,
            WIDTH: 59.5,
            DEPTH: 60.5,
            NOISE: 76,
            SALES: {
                "https://www.boulanger.com/ref/1175369": # LINK
                    630, # PRICE
            },
        },
    },
    "Essentielb": {
        "ELS149-1b":
        {
            DRUM: 86,
            CAPACITY: 14,
            HEIGHT: 85,
            WIDTH: 60,
            DEPTH: 67,
            NOISE: 80,
            SALES: {
                "https://www.boulanger.com/ref/8010633": # LINK
                    549, # PRICE
            },
        },
        "ELF1414-1b":
        {
            DRUM: 85,
            CAPACITY: 14,
            HEIGHT: 85,
            WIDTH: 60,
            DEPTH: 67,
            NOISE: 80,
            SALES: {
                "https://www.boulanger.com/ref/8010630": # LINK
                    649, # PRICE
            },
        },
        "ELF1214-2b":
        {
            DRUM: 85,
            CAPACITY: 12,
            HEIGHT: 85,
            WIDTH: 60,
            DEPTH: 67,
            NOISE: 80,
            SALES: {
                "https://www.boulanger.com/ref/8010598": # LINK
                    549, # PRICE
            },
        },
    },
    "Indesit": {
        "BDE107436WKVFR":
        {
            DRUM: 71,
            CAPACITY: 10,
            HEIGHT: 85,
            WIDTH: 59.5,
            DEPTH: 60.5,
            NOISE: 79,
            SALES: {
                "https://www.boulanger.com/ref/1195720": # LINK
                    649, # PRICE
            },
        },
        "BWE101496XWKVFR":
        {
            DRUM: 71,
            CAPACITY: 10,
            HEIGHT: 85,
            WIDTH: 59.5,
            DEPTH: 60.5,
            NOISE: 76,
            SALES: {
                "https://www.boulanger.com/ref/1195727": # LINK
                    499, # PRICE
            },
        },
    },
    "HAIER": {
        "HW120-B14979-FR":
        {
            DRUM: 78,
            CAPACITY: 12,
            HEIGHT: 85,
            WIDTH: 60,
            DEPTH: 69,
            NOISE: 69,
            SALES: {
                "https://www.boulanger.com/ref/1144611": # LINK
                    576, # PRICE
            },
        },
    },
    "Samsung": {
        "WW11BGA046AE":
        {
            DRUM: 72,
            CAPACITY: 11,
            HEIGHT: 85,
            WIDTH: 60,
            DEPTH: 63.7,
            NOISE: 72,
            SALES: {
                "https://www.boulanger.com/ref/1180393": # LINK
                    549, # PRICE
            },
        },
        "WW11BB504DAES3":
        {
            DRUM: 71,
            CAPACITY: 11,
            HEIGHT: 85,
            WIDTH: 60,
            DEPTH: 63.7,
            NOISE: 72,
            SALES: {
                "https://www.electrodepot.fr/lave-linge-frontal-samsung-ww11bb504daes3-11-1400.html": # LINK
                    500, # PRICE
            },
        },
    },
    "Valberg": {
        "WF 1214 A W566C":
        {
            DRUM: 76,
            CAPACITY: 12,
            HEIGHT: 85,
            WIDTH: 59.5,
            DEPTH: 59.5,
            NOISE: 80,
            SALES: {
                "https://www.electrodepot.fr/lave-linge-frontal-valberg-wf-1214-a-w566c.html": # LINK
                    390, # PRICE
            },
        },
        "WF 1810 D W566C":
        {
            DRUM: 127,
            CAPACITY: 18,
            HEIGHT: 101,
            WIDTH: 68.6,
            DEPTH: 86.4,
            NOISE: 80,
            SALES: {
                "https://www.electrodepot.fr/lave-linge-frontal-valberg-wf-1810-d-s566c.html": # LINK
                    680, # PRICE
            },
        },
    },
    "Vedette": {
        "LFVQ124W":
        {
            DRUM: 76,
            CAPACITY: 12,
            HEIGHT: 85,
            WIDTH: 59.5,
            DEPTH: 59.5,
            NOISE: 78,
            SALES: {
                "https://www.darty.com/nav/achat/gros_electromenager/lavage_sechage/lave-linge_hublot/vedette_lfvq124w.html": # LINK
                    543, # PRICE
            },
        },
    },
    "Beko": {
        "LLF11W2":
        {
            DRUM: 72,
            CAPACITY: 11,
            HEIGHT: 84,
            WIDTH: 60,
            DEPTH: 67,
            NOISE: 74,
            SALES: {
                "https://www.darty.com/nav/achat/gros_electromenager/lavage_sechage/lave-linge_hublot/beko_llf11w2.html": # LINK
                    462, # PRICE
            },
        },
    },
    "": {
        "":
        {
            DRUM: 0,
            CAPACITY: 0,
            HEIGHT: 0,
            WIDTH: 0,
            DEPTH: 0,
            NOISE: 0,
            SALES: {
                "": # LINK
                    0, # PRICE
            },
        },
    },
}

drum2tm_model = dict()
for (tm, models) in PRODUCTS.items():
    for (model, details) in models.items():
        d = details[DRUM]
        tm_model = (tm, model)
        if d in drum2tm_model:
            drum2tm_model[d].append(tm_model)
        else:
            a = list()
            a.append(tm_model)
            drum2tm_model[d] = a

drum_sorted = list(drum2tm_model)
drum_sorted.sort()
for d in drum_sorted:
    print(d)
    for tm_model in drum2tm_model[d]:
        details = PRODUCTS[tm_model[0]][tm_model[1]]
        print(f'    {tm_model}: {details[CAPACITY]}kg {details[NOISE]}dB {details[WIDTH]}x{details[HEIGHT]}x{details[WIDTH]}')
        prices = dict()
        for (link, price) in details[SALES].items():
            if price in prices:
                prices[price].append(link)
            else:
                a = list()
                a.append(link)
                prices[price] = a
        prices_sorted = list(prices)
        prices_sorted.sort()
        for price in prices_sorted:
            print(f'        {price}: {prices[price]}')
