version = v1.0.1
packageDir="package-${version}"

all: akfs akps akfile aknet akagent build-package

.PHONY: akfs akps akfile aknet akagent build-package

build-package:
	test -d $(packageDir) || mkdir -p $(packageDir)
	install -p -D -m 0755 ./akfs/akfs.ko $(packageDir)
	install -p -D -m 0755 ./akps/process.ko $(packageDir)
	install -p -D -m 0755 ./aknet/net.ko $(packageDir)
	install -p -D -m 0755 ./akfile/file.ko $(packageDir)
	
	install -p -D -m 0755 ./akagent/akagent $(packageDir)
	install -p -D -m 0755 ./akagent/*.json $(packageDir)
	install -p -D -m 0755 ./akagent/*.filter $(packageDir)
	install -p -D -m 0755 ./akagent/Dockerfile $(packageDir)

	install -p -D -m 0755 install.sh $(packageDir)
	tar -czf  $(packageDir).tar.gz  $(packageDir)
	echo "build package. new package is "$(packageDir).tar.gz

akfs:
	make -C ./akfs/
	echo "build akfs done"


akps:
	make -C ./akps/
	echo "build akps done"
akfile:
	make -C ./akfile/
	echo "build akfile done"
aknet:
	make -C ./aknet/
	echo "build aknet done"


akagent:
	
	make -C ./akagent/
	echo "build akagent done"
