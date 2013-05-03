
import MySQLdb

db = MySQLdb.connect(user = 'eiger', passwd = 'eigerdreams', db = 'eiger_development')
cursor = db.cursor()

#
# GPGPU-3 data
#

static_data = {
	'cp' : [1112592, 0, 0, 20, 11, 153506, 1705, 0, 110, 2640, 0, 0, 880, 385, 330, 0, 440, 0],
	'mri-fhd' : [582630, 0, 0, 18.5, 17, 46501, 760, 298, 206, 420, 20, 25, 69, 56, 278, 0, 0, 0],
	'mri-q' : [517229, 0, 0, 17, 11, 67397, 461, 178, 118, 245, 0, 15, 43, 33, 168, 0, 0, 0],
	'pns2000' : [720002676, 19, 23.15, 35, 224, 71.42, 471, 82, 80, 22, 0, 0, 39, 42, 115, 19, 0, 0],
	'rpes' : [59883768, 5, 4.6, 27, 6, 11537369, 2226, 1267, 280, 14632, 0, 440, 113, 1605, 2102, 167, 495, 0],
	'sad' : [8948776, 2, 5.5, 21, 3, 2999568, 810, 140, 71, 93, 0, 0, 33, 99, 91, 6, 0, 0],
	'tpacf' : [5009948, 4, 14.5, 22, 3, 1660172, 224, 36, 34, 9, 0, 3, 7, 32, 67, 4, 0, 0],
	'nbody256' : [20528, 2, 18, 27, 4, 8200, 68, 3, 5, 77, 0, 0, 5, 24, 13, 2, 4, 0],
	'nbody512' : [36912, 2, 18, 27, 4, 16392, 68, 3, 5, 77, 0, 0, 5, 24, 13, 2, 4, 0],
	'nbody1024' : [69680, 2, 18, 27, 4, 32776, 68, 3, 5, 77, 0, 0, 5, 24, 13, 2, 4, 0],
	'nbody2048' : [135216, 2, 18, 27, 4, 65544, 68, 3, 5, 77, 0, 0, 5, 24, 13, 2, 4, 0],
	'nbody4096' : [266288, 2, 18, 27, 4, 131080, 68, 3, 5, 77, 0, 0, 5, 24, 13, 2, 4, 0],
	'pns250' : [749387124, 19, 23.15, 35, 2, 1000, 471, 82, 80, 22, 0, 0, 39, 42, 115, 19, 0, 0],
	'pns500' : [747504924, 19, 23.15, 35, 4, 1000, 471, 82, 80, 22, 0, 0, 39, 42, 115, 19, 0, 0],
	'pns1000' : [740003124, 19, 23.15, 35, 27, 285.3, 471, 82, 80, 22, 0, 0, 39, 42, 115, 19, 0, 0],
	'pns1500' : [742502796, 19, 23.15, 35, 73, 132, 471, 82, 80, 22, 0, 0, 39, 42, 115, 19, 0, 0],
	'hotspot1' : [123144, 3, 19.66, 23, 3, 40000, 127, 0, 26, 10, 15, 0, 3, 34, 37, 3, 0, 0],
	'hotspot2' : [123144, 3, 19.66, 23, 3, 40000, 127, 0, 26, 10, 15, 0, 3, 34, 37, 3, 0, 0],
	'hotspot3' : [123144, 3, 19.66, 23, 3, 40000, 127, 0, 26, 10, 15, 0, 3, 34, 37, 3, 0, 0],
	'hotspot4' : [123144, 3, 19.66, 23, 3, 40000, 127, 0, 26, 10, 15, 0, 3, 34, 37, 3, 0, 0],
	'hotspot5' : [123144, 3, 19.66, 23, 3, 40000, 127, 0, 26, 10, 15, 0, 3, 34, 37, 3, 0, 0],
	'hotspot6' : [123144, 3, 19.66, 23, 3, 40000, 127, 0, 26, 10, 15, 0, 3, 34, 37, 3, 0, 0],
	'hotspot7' : [123144, 3, 19.66, 23, 3, 40000, 127, 0, 26, 10, 15, 0, 3, 34, 37, 3, 0, 0],
	'lu32' : [5904, 6, 8.33, 14.66, 2, 4096, 1647, 0, 34, 48, 0, 0, 255, 358, 93, 14, 0, 0],
	'lu1024' : [4196362, 6, 8.33, 14.66, 2, 4194304, 1647, 0, 34, 48, 0, 0, 255, 358, 93, 14, 0, 0 ]
}
estimatable_data = {
	'cp' : [100, 0.01, 49.2, 0, 128, 256],
	'mri-fhd' : [100, 0.06, 49.6, 0, 292.57, 110.57],
	'mri-q' : [100, 0.04, 49.6, 0, 320, 97.5],
	'pns2000' : [97.2, 4.64, 65.6, 51.5, 248.88, 17.99],
	'rpes' : [63.85, 2.74, 73.1, 76.6, 40.58, 64757],
	'sad' : [95.4, 5.88, 47.7, 2.9, 70.28, 594],
	'tpacf' : [80.51, 0.01, 48.1, 12.4, 206.11, 156.63],
	'nbody256' : [100, 0.0817394, 49.6, 66.4062, 256, 1],
	'nbody512' : [100, 0.0492813, 49.6, 66.4062, 256, 2, ],
	'nbody1024' : [100, 0.032934, 49.6, 66.4062, 256, 4, ],
	'nbody2048' : [100, 0.0247305, 49.6, 66.4062, 256, 8, ],
	'nbody4096' : [100, 0.0206213, 49.6, 66.4062, 256, 16],
	'pns250' : [42.45, 2.06, 60.2, 57.3, 108.684, 249.981],
	'pns500' : [72.94, 4.47, 66.7, 54.5, 186.737, 249.986],
	'pns1000' : [90.9168, 4.76, 64.5, 52.6, 232.746, 71.4228],
	'pns1500' : [95.62, 4.58, 66.7, 52.14, 244.803, 32.9982],
	'hotspot1' : [80.5, 1.2, 49.4, 56.5, 205.87, 63.95],
	'hotspot2' : [74.14, .85, 44.4, 56.6, 189.69, 80.95],
	'hotspot3' : [70.9, .69, 42.3, 56.6, 181.87, 99.909],
	'hotspot4' : [62.66, .56, 41.6, 56.7, 160.25, 168.95],
	'hotspot5' : [57.89, .439, 41.5, 56.7, 148.054, 288.92],
	'hotspot6' : [54.11, .375, 42.03, 56.8, 139.93, 624.661],
	'hotspot7' : [50.6, .328, 42.39, 56.8, 133.36, 2499.73],
	'lu32' : [47.49, 2.42, 23.14, 44.67, 71.62, 1],
	'lu1024' : [76.366, 3.823, 55.8, 91.45, 92.64, 460.12]
}
dynamic_data = {
	'cp' : [225955840, 0, 112668160, 2929259520, 0, 0, 450560, 450644480, 112780800, 0, 901120000, 0, 10],
	'mri-fhd' : [37218288, 13737984, 26135124, 213326356, 11010048, 13737984, 38136, 13800780, 45257916, 0, 0, 0, 7],
	'mri-q' : [22805424, 7852032, 12572208, 118778636, 0, 7850496, 10896, 6302400, 26640828, 0, 0, 0, 4],
	'pns2000' : [52468760284, 24228698088, 5628012088, 21047223288, 0, 0, 5339154376, 482780344, 5744498754, 41073008, 0, 0, 112],
	'rpes' : [21425348430, 8929960620, 5008934250, 41965768220, 0, 1211896780, 687860280, 14312647184, 15469148966, 2651429220, 999566552, 0, 71],
	'sad' : [15161256, 593802, 600930, 6044544, 0, 0, 191268, 4498362, 822294, 19008, 0, 0, 3],
	'tpacf' : [1348773894, 221533344, 208574048, 60892128, 0, 140980224, 255714, 322229316, 842506252, 20160936, 0, 0, 1],
	'nbody256' : [11672166, 30906, 700536, 39085788, 0, 0, 51510, 8035560, 782952, 20604, 2637312, 0, 101],
	'nbody512' : [46101450, 123624, 2760936, 156095904, 0, 0, 123624, 31998012, 3018486, 82416, 10549248, 0, 101],
	'nbody1024' : [183231372, 494496, 10961328, 623889120, 0, 0, 329664, 127703592, 11847300, 329664, 42196992, 0, 101],
	'nbody2048' : [730576632, 1977984, 43680480, 2494567488, 0, 0, 988992, 510237456, 46935912, 1318656, 168787968, 0, 101],
	'nbody4096' : [2917608816, 7911936, 174392256, 9976291968, 0, 0, 3296640, 2039796000, 186837072, 5274624, 675151872, 0, 101],
	'pns250' : [1637900, 605250, 241250, 249250, 0, 0, 70250, 330140, 263019, 6500, 0, 0, 1],
	'pns500' : [11052772, 4611029, 1405441, 3136075, 0, 0, 85754, 1069906, 1509939, 33558, 0, 0, 2],
	'pns1000' : [327332366, 147471912, 37223742, 120998430, 0, 0, 31260828, 10649895, 38898093, 572316, 0, 0, 13],
	'pns1500' : [1457230570, 672610224, 159702576, 3629952900, 0, 0, 146608176, 22849853, 163863150, 1473120, 0, 0, 36],
	'hotspot1' : [37491200, 0, 7433600, 3232000, 4848000, 0, 969600, 10019200, 11877600, 646400, 0, 0, 100],
	'hotspot2' : [16110900, 0, 3511350, 1652400, 3098250, 0, 309825, 4234275, 6517800, 413100, 0, 0, 50],
	'hotspot3' : [11651772, 0, 2675220, 1307584, 2673960, 0, 178392, 3031884, 5242556, 356600, 0, 0, 34],
	'hotspot4' : [12962300, 0, 3075800, 1537900, 3295500, 0, 164775, 3350425, 6227650, 439400, 0, 0, 25],
	'hotspot5' : [16750440, 0, 4066230, 2063460, 4551750, 0, 182070, 4308990, 8400210, 606900, 0, 0, 20],
	'hotspot6' : [30093938, 0, 7442100, 3806718, 8560545, 0, 285561, 7713345, 15574465, 1145000, 0, 0, 17],
	'hotspot7' : [106063102, 0, 26550200, 13695972, 31239930, 0, 892743, 27110425, 56139216, 4175000, 0, 0, 15],
	'lu32' : [11466, 0, 2337, 3972, 0, 0, 400, 6514, 3353, 163, 0, 0, 4],
	'lu1024' : [922431904, 0, 79575648, 351955776, 0, 0, 68845696, 674142496, 115130688, 12808960, 0, 0, 190]
}

times = { 
	'8600GS' : {
		'cp' : 3.460000,
		'mri-fhd' : 1.940000,
		'mri-q' : 1.360000,
		'pns2000' : 18.307000,
		'rpes' : 4.144000,
		'sad' : 1.157000,
		'tpacf' : 15.704000,
		'nbody256' : 0.019789,
		'nbody512' : 0.035413,
		'nbody1024' : 0.119982,
		'nbody2048' : 0.463266,
		'nbody4096' : 1.834434,
		'pns250' : 3.012400,
		'pns500' : 3.043500,
		'pns1000' : 5.285300,
		'pns1500' : 9.332900,
		'hotspot1' : 2.833300,
		'hotspot2' : 2.366300,
		'hotspot3' : 3.105800,
		'hotspot4' : 2.374100,
		'hotspot5' : 2.272900,
		'hotspot6' : 2.195100,
		'hotspot7' : 2.405200,
		'lu32' : 3.144700,
		'lu1024' : 6.974400
		},		
	'8800GTX' : {
		'cp' : 0.568800,
		'mri-fhd' : 0.615000,
		'mri-q' : 0.441000,
		'pns2000' : 2.718000,
		'rpes' : 0.834000,
		'sad' : 0.619000,
		'tpacf' : 2.540000,
		'nbody256' : 0.014770,
		'nbody512' : 0.025990,
		'nbody1024' : 0.048417,
		'nbody2048' : 0.093310,
		'nbody4096' : 0.183316,
		'pns250' : 0.539980,
		'pns500' : 0.545560,
		'pns1000' : 0.947410,
		'pns1500' : 1.672960,
		'hotspot1' : 0.507890,
		'hotspot2' : 0.424170,
		'hotspot3' : 0.556720,
		'hotspot4' : 0.425570,
		'hotspot5' : 0.407430,
		'hotspot6' : 0.393470,
		'hotspot7' : 0.431150,
		'lu32' : 0.563700,
		'lu1024' : 1.250190
		},	
	'280GTX' : {
		'cp' : 0.428390,
		'mri-fhd' : 0.676520,
		'mri-q' : 0.473940,
		'pns2000' : 2.375040,
		'rpes' : 0.616150,
		'sad' : 0.622380,
		'tpacf' : 1.852810,
		'nbody256' : 0.018338,
		'nbody512' : 0.032198,
		'nbody1024' : 0.053831,
		'nbody2048' : 0.102090,
		'nbody4096' : 0.199816,
		'pns250' : 0.424000,
		'pns500' : 0.477000,
		'pns1000' : 0.738000,
		'pns1500' : 1.243000,
		'hotspot1' : 0.365000,
		'hotspot2' : 0.290000,
		'hotspot3' : 0.309000,
		'hotspot4' : 0.291000,
		'hotspot5' : 0.281000,
		'hotspot6' : 0.327000,
		'hotspot7' : 0.305000,
		'lu32' : 0.416000,
		'lu1024' : 0.893000
		},	
	'C1060' : {
		'cp' : 0.365640,
		'mri-fhd' : 0.621820,
		'mri-q' : 0.418890,
		'pns2000' : 2.078190,
		'rpes' : 0.563690,
		'sad' : 0.579000,
		'tpacf' : 1.772810,
		'nbody256' : 0.017931,
		'nbody512' : 0.029107,
		'nbody1024' : 0.051595,
		'nbody2048' : 0.096574,
		'nbody4096' : 0.186588,
		'pns250' : 0.387000,
		'pns500' : 0.391000,
		'pns1000' : 0.679000,
		'pns1500' : 1.199000,
		'hotspot1' : 0.364000,
		'hotspot2' : 0.304000,
		'hotspot3' : 0.399000,
		'hotspot4' : 0.305000,
		'hotspot5' : 0.292000,
		'hotspot6' : 0.282000,
		'hotspot7' : 0.309000,
		'lu32' : 0.404000,
		'lu1024' : 0.896000
		},	
	'atom' : {
		'cp' : 1027.6415,
		'mri-fhd' : 19.5707,
		'mri-q' : 16.4012,
		'pns2000' : 592.7248,
		'rpes' : 1184.1983,
		'sad' : 14.9005,
		'tpacf' : 668.0902,
		'nbody256' : 1.3739,
		'nbody512' : 3.3997,
		'nbody1024' : 13.4116,
		'nbody2048' : 53.5904,
		'nbody4096' : 213.7070,
		'pns250' : 12.9990,
		'pns500' : 23.0300,
		'pns1000' : 181.0320,
		'pns1500' : 350.4700,
		'hotspot1' : 3.2680,
		'hotspot2' : 3.7400,
		'hotspot3' : 4.0400,
		'hotspot4' : 4.5170,
		'hotspot5' : 5.7770,
		'hotspot6' : 9.9530,
		'hotspot7' : 32.4980,
		'lu32' : 18.1670,
		'lu1024' : 48.2480
		},
	'phenom' : {
		'cp' : 60.46820,
		'mri-fhd' : 4.93686,
		'mri-q' : 4.08240,
		'pns2000' : 204.72076,
		'rpes' : 110.77504,
		'sad' : 2.42777,
		'tpacf' : 120.41092,
		'nbody256' : 0.29740,
		'nbody512' : 0.58626,
		'nbody1024' : 1.54796,
		'nbody2048' : 5.91421,
		'nbody4096' : 20.49080,
		'pns250' : 2.77800,
		'pns500' : 3.30600,
		'pns1000' : 10.24400,
		'pns1500' : 82.96100,
		'hotspot1' : 0.66800,
		'hotspot2' : 0.76800,
		'hotspot3' : 0.68100,
		'hotspot4' : 0.85000,
		'hotspot5' : 1.25100,
		'hotspot6' : 1.67200,
		'hotspot7' : 5.08600,
		'lu32' : 4.43700,
		'lu1024' : 113.75400
		},
	'nehalem' : {
		'cp' : 53.97019,
		'mri-fhd' : 2.10587,
		'mri-q' : 2.53092,
		'pns2000' : 59.53154,
		'rpes' : 49.06124,
		'sad' : 1.41763,
		'tpacf' : 62.81561,
		'nbody256' : 0.17649,
		'nbody512' : 0.31495,
		'nbody1024' : 1.29162,
		'nbody2048' : 4.38914,
		'nbody4096' : 13.98090,
		'pns250' : 1.80000,
		'pns500' : 2.53700,
		'pns1000' : 5.62900,
		'pns1500' : 16.94700,
		'hotspot1' : 0.44700,
		'hotspot2' : 0.44700,
		'hotspot3' : 0.44000,
		'hotspot4' : 0.63200,
		'hotspot5' : 0.86400,
		'hotspot6' : 1.26000,
		'hotspot7' : 2.90100,
		'lu32' : 0.71000,
		'lu1024' : 2.01600
		}
 }

#
# Create application metrics
#
static_metrics = ['Extent of Memory', 'Context Switches', 'Live Registers', 
	'Registers/Thread', 'DMAs', 'DMA Size', 'Static Integer arithmetic', 
	'Static Integer logical', 'Static Integer comparison', 
	'Static Float single', 'Static Float double', 'Static Float comparison',
	'Static Memory offchip', 'Static Memory onchip', 'Static Control',
	'Static Parallelism', 'Static Special', 'Static Other']

estimatable_metrics = ['Activity Factor', 'Memory Intensity', 
	'Memory Efficiency', 'Memory Sharing', 'SIMD', 'MIMD']
dynamic_metrics = ['Dynamic Integer arithmetic', 'Dynamic Integer logical',
	'Dynamic Integer comparison', 'Dynamic Float single',
	'Dynamic Float double', 'Dynamic Float comparison',	
	'Dynamic Memory offchip', 'Dynamic Memory onchip', 'Dynamic Control',
	'Dynamic Parallelism', 'Dynamic Special', 'Dynamic Other', 'Kernels']
machine_metrics = ['Clock Frequency', 'Hardware Threads Per Core', 'Cores',
	'Memory Bandwidth Per Controller', 'Memory Controllers', 'Issue Width',
	'l2 cache', 'Out Of Order', 'Warp Size']

result_metrics = ['Runtime',]

staticMetricId = 1
cursor.executemany(
	"""INSERT INTO eiger_metric(type, name, description) VALUES (%s, %s, %s)""",
	[('static', metric, 'gpgpu-3 metric') for metric in static_metrics])

estimableMetricId = cursor.lastrowid + 1
cursor.executemany(
	"""INSERT INTO eiger_metric(type, name, description) VALUES (%s, %s, %s)""",
	[('dynamic', metric, 'gpgpu-3 metric') for metric in estimatable_metrics])

dynamicMetricId = cursor.lastrowid + 1
cursor.executemany(
	"""INSERT INTO eiger_metric(type, name, description) VALUES (%s, %s, %s)""",
	[('dynamic', metric, 'gpgpu-3 metric') for metric in dynamic_metrics])

resultMetricId = cursor.lastrowid + 1
cursor.executemany(
	"""INSERT INTO eiger_metric(type, name, description) VALUES (%s, %s, %s)""",
	[('result', metric, 'gpgpu-3 result metric') for metric in result_metrics])
resultMetricId = cursor.lastrowid	

#
# create machine metrics
#
cursor.executemany(
	"""INSERT INTO eiger_metric(type, name, description) VALUES (%s, %s, %s)""",
	[('machine', metric, 'gpgpu-3 metric') for metric in machine_metrics])


#
# Insert machine data
#
gpu_machine_data = {
	'8800GTX' : [1.5, 24, 16, 14.3, 6, 1, 0, 0, 32],
	'C1060' : [1.3, 24, 30, 12.75, 8, 1, 0, 0, 32],
	'280GTX' : [1.3, 24, 30, 17.62, 8, 1, 0, 0, 32],
	'8600GS' : [1.2, 24, 2, 5.6, 2, 1, 0, 0, 32]}
cpu_machine_data = {
	'nehalem' : [2.6, 2, 4, 8.53, 3, 4, 512, 1, 1],
	'atom' : [1.6, 2, 1, 3.54, 1, 2, 512, 0, 1],
	'phenom' : [2.2, 1, 4, 8.53, 2, 3, 512, 1, 1]}

machineToIdMap = {}
for series in [gpu_machine_data, cpu_machine_data]:
	for processor, metrics in series.items():
		cursor.execute("INSERT INTO eiger_machine(name, description) VALUES ('%s', 'processor')" % (processor,))
		machineID = cursor.lastrowid
		machineToIdMap[processor] = machineID
		cursor.executemany("""INSERT INTO eiger_machine_map(machineID, metricID, metric) VALUES (%s,%s,%s)""", 
			[(machineID, i+1, metric) for i, metric in enumerate(metrics)])

#
# Insert applications
#

applications = ['cp', 'mri-fhd', 'mri-q', 'pns2000', 'rpes', 'sad', 'tpacf', 'nbody256', \
	'nbody512', 'nbody1024', 'nbody2048', 'nbody4096', 'pns250', 'pns500', 'pns1000', 'pns1500', \
	'hotspot1', 'hotspot2', 'hotspot3', 'hotspot4', 'hotspot5', 'hotspot6', 'hotspot7', 'lu32', 'lu1024' ]

cursor.executemany(
	"""INSERT INTO eiger_application(name, description) VALUES (%s, %s)""",
	[(a,'gpgpu-3 application') for a in applications])

#
# Insert trials
#

cursor.execute("""INSERT INTO eiger_datacollection(name, description, created) values
	('gpgpu3', 'Data gathered for GPGPU3 paper', now())""")
dataCollectionID = cursor.lastrowid

applicationToTrialID = {}
applicationToDatasetID = {}
for app, metric in static_data.items():
	cursor.execute("INSERT INTO eiger_application(name, description) VALUES ('%s', '%s')" % (app, 'gpgpu3 application'))
	applicationToTrialID[app] = cursor.lastrowid
	cursor.execute("""INSERT INTO eiger_dataset(applicationID, name, description, created, url) VALUES
		('%s', '%s', 'Default data set', now(), 'database.trace')""" % (applicationToTrialID[app], app))
	applicationToDatasetID[app] = cursor.lastrowid

# static metrics
for app, metricData in static_data.items():
	for i, datum in enumerate(metricData):
		cursor.execute("""INSERT INTO eiger_static_metric(datasetID, metricID, metric) VALUES
			(%s,%s, %s)""" % (applicationToDatasetID[app], staticMetricId  + i, datum))

# estimable metrics
for app, metricData in estimatable_data.items():
	for i, datum in enumerate(metricData):
		cursor.execute("""INSERT INTO eiger_static_metric(datasetID, metricID, metric) VALUES
			(%s,%s,%s)""" % (applicationToDatasetID[app], estimableMetricId  + i, datum))

# dynamic metrics
for app, metricData in dynamic_data.items():
	for i, datum in enumerate(metricData):
		cursor.execute("""INSERT INTO eiger_static_metric(datasetID, metricID, metric) VALUES
			(%s,%s,%s)""" % (applicationToDatasetID[app], dynamicMetricId  + i, datum))

# trial times
for machineName, results in times.items():
	if machineName in ('8800GTX', 'C1060', '280GTX', '8600GS' ):
		for app, time in results.items():
			propertiesID = 0
			machineID = machineToIdMap[machineName]
			datasetID = applicationToDatasetID[app]
			applicationID = applicationToTrialID[app]
			cursor.execute("""INSERT INTO eiger_trial(dataCollectionID, machineID, applicationID, datasetID, propertiesID)
				VALUES (%s, %s, %s, %s, %s)""" % (dataCollectionID, machineID, applicationID, datasetID, propertiesID))
			trialID = cursor.lastrowid
		
			cursor.execute(query = """INSERT INTO eiger_dynamic_metric(trialID, metricID, metric) VALUES
				(%s, %s, %s)""" % (trialID, resultMetricId, time))

#
#
#

