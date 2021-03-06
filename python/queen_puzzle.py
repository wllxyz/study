#!/usr/bin/python
import sys

def is_ok(queens):
	m = [([0] * 8) for i in range(8)]
	i = 0
	for q in queens:
		if m[i][q] == 0:
			for j in range(8):
				if m[i][j] == 1 or m[j][q] == 1:
					return 0
				else:
					m[i][j] = 2
					m[j][q] = 2
			#######################################
			if i > q:
				j = i - q
				k = 0
				for l in range(8-j):
					if m[j+l][k+l] == 1:
						return 0
					else:
						m[j+l][k+l] = 2
			else:
				j = 0
				k = q - i
				for l in range(8-k):
					if m[j+l][k+l] == 1:
						return 0
					else:
						m[j+l][k+l] = 2
			#######################################
			n = i + q
			if n < 8:
				j = n
				k = 0
				for l in range(n+1):
					if m[j-l][k+l] == 1:
						return 0
					else:
						m[j-l][k+l] = 2
			else:
				j = 7
				k = n -7
				for l in range(8-k):
					if m[j-l][k+l] == 1:
						return 0
					else:
						m[j-l][k+l] = 2
			#######################################
			m[i][q] = 1	
		else:
			return 0
		i = i+1
	return 1

def search_queens(queens):
	if is_ok(queens):
		if len(queens)==8:
			return 1
		else:
			queens.append(0)
			return 0
	else:
		while queens[len(queens)-1] == 7:
			queens.pop()
		queens[len(queens)-1] += 1
		return 0
	

def queen_puzzle():
	queens = []
	steps = 0
	while search_queens(queens)==0:
		print queens
		steps += 1

	for i in range(8):
		for j in range(8):
			if queens[i] == j:
				print ' *',
			else:
				print ' -',
		print
	print 'total steps=',steps

if __name__ == '__main__':
	queen_puzzle()
