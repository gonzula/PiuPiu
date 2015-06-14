import random
import string

linhas = []
linhas.append("1")

for i in xrange(1, 15 + 1):
    linhas.append('@user%03d' % i)
    linhas.append(''.join(random.choice(string.ascii_uppercase + string.digits + string.ascii_lowercase) for _ in range(random.randrange(10, 340))))
    linhas.append(''.join(random.choice(string.ascii_uppercase + string.digits + string.ascii_lowercase) for _ in range(random.randrange(10, 20))))
    linhas.append(str(random.randrange(10000)))
    linhas.append(''.join(random.choice(string.ascii_uppercase + string.digits + string.ascii_lowercase) for _ in range(random.randrange(10, 20))))
    linhas.append(str(random.randrange(10000)))
    linhas.append(str(random.randrange(10000000000)))
linhas.append('')
linhas.append('')
linhas.append('0')
linhas.append('')

print '\n'.join(linhas)

