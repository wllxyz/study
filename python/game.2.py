#!/usr/bin/python
class Man:
	def play(self,d):
		print 'choose number in ',d,':'
		n = input()
		while n not in d:
			print 'choose number in',d,':'
			n = input()
		print n,'==>?:(0<=m and m<',n,')'
		m = input()
		while not (0<=m and m<n):
			print n,'==>?:(0<=m and m<',n,')'
			m = input()
		i=0
		for e in d:
			if e==n:
				if m==0:
					d.remove(n)
					break
				else:
					d[i]=m
					break
			i=i+1

class Robot:

	def RemoveOne(self,d):
		for e in d:
			if e==1:
				d.remove(e)
				break
	
	def RemoveTwo(self,d):
		for e in d:
			if e!=1:
				d.remove(e)
				break
	
	def Two2One(self,d):
		for i in range(len(d)):
			if d[i]!=1:
				d[i]=1
				break
	
	def play(self,d):
		if len(d)==0:
			print 'lose'
		else:
			ones = 0
			twos = 0
			for e in d:
				if e==1:
					ones += 1
				else:
					twos += 1
			if twos==0:
				if ones%2 == 0:
					self.RemoveOne(d)
					print 'will lose'
				else:
					self.RemoveOne(d)
					print 'will win'
			else:
				if twos%2 == 0:
					if ones%2 == 0:
						self.Two2One(d)
						print 'will lose'
					else:
						self.RemoveOne(d)
						print 'will win'
				else:
					if ones%2 == 0:
						self.RemoveTwo(d)
						print 'will win'
					else:
						self.Two2One(d)
						print 'will win'
					

def CreateGame(d):
	print 40*'>'
	n = input('n=?')
	for i in range(n):
		print 'number ',i,'=:'
		m = input()
		d.append(m)
	print 'Game created'
	print 40*'<'


def CreatePlayer(text):
	print text, '0-Robot or 1-Man?'
	type = input()
	if type==1:
		return Man()
	else:
		return Robot()

if __name__ == '__main__':
	while 1: 
		d = []
		CreateGame(d)
		red = CreatePlayer('red')
		blue = CreatePlayer('blue')
		turn = 0
		while len(d)!=0:
			print 40*'+'
			print d
			if turn==0:
				print 'red move:'
				red.play(d)
				turn = 1
			else:
				print 'blue move'
				blue.play(d)
				turn = 0
		print 40*'*'
		print d
		if turn==0:
			print 'blue win'
		else:
			print 'red win'
		print 'game over'
		c = input('continue?1--yes 0--no')
		if c==0:
			break
	
#mvc
#model		game data
#view		game ui
#control	game input
#state machine
#options: set player, set computer AI level
#player move

