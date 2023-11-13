class IVec2:
    def __init__(self, x : int, y : int):
        self.x = x
        self.y = y
    
    def __add__(self, other):
        return IVec2(self.x + other.x, self.y + other.y)
    
    def __sub__(self, other):
        return IVec2(self.x - other.x, self.y - other.y)
    
    def Magnitude(self):
        return (self.x ** 2 + self.y ** 2) ** 0.5
    
    def Dist(self, other):
        return (other - self).Magnitude()

def GetInt():
    try:
        return int(input(""))
    except:
        return GetInt()

print("Input width ")
width = GetInt()
print("Input height ")
height = GetInt()

dimensions = IVec2(width, height)
totalCells = width * height

def FindValue(a : IVec2, b : IVec2):
    return a.Dist(b)

for i in range(totalCells * (totalCells - 1)):
    