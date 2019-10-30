import tensorflow as tf
import os
os.environ['TF_CPP_MIN_LOG_LEVEL']='2'

'''
  BASIC VECTOR
'''

a = tf.zeros([2,3],tf.int32,name='zeroVector')
b = tf.ones([2,3])
c = tf.fill([2,3],8)

with tf.Session() as session: 
    writer = tf.summary.FileWriter('./graphs',session.graph)

    print (session.run(a))
    print (session.run(b))
    print (session.run(c))

writer.close()

'''
  SEQUENCE 
'''
# LINSPACE => linspace(start, stop, num, name)
# create a num of sequence with even space
l = tf.linspace(10.0,13.0,2, name='linSpace')

# RANGE => range(start, limit, delta, dtype, name)
# create a sequence extends by increments of delta (but not exceed the limits)
a = tf.range(3,18,3)
b = tf.range(3,1,-.5)
c = tf.range(5) # this mean limit is 5

with tf.Session() as session: 
    writer = tf.summary.FileWriter('./graphs', session.graph)

    print (session.run(l))
    print (session.run(a),session.run(b),session.run(c))

writer.close()

'''
  RANDOM
'''

a = tf.random_normal([2,4], mean = 10.0, dtype=tf.float32)
d = tf.random_shuffle([0,1,2,3,4,5,6,7,8,9])

with tf.Session() as session: 
    writer = tf.summary.FileWriter('./graphs', session.graph)

    print (session.run(a))
    print (session.run(d))

writer.close()

'''
  Math Operation
'''

a = tf.constant([3,16])
b = tf.constant([2,2])

with tf.Session() as session: 
    writer = tf.summary.FileWriter('./graphs', session.graph)

    print (session.run(tf.add(a,b)))
    print (session.run(tf.add_n([a,b,b])))

    print (session.run(tf.multiply(a,b))) 
    print (session.run(tf.div(a,b)))
    print (session.run(tf.mod(a,b)))

    print (session.run(tf.matmul(a, tf.transpose(b)))) 
    # Will throw error Shape must be rank 2

    print (session.run(tf.matmul(tf.reshape(a,shape=[1,2]), tf.reshape(b, shape=[2,1]))))

writer.close()