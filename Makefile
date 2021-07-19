version = v1.0.1
packageDir="package-${version}"
akagentDockerPkg="akagent-docker-$(version).tar"

all: akfs akps akfile aknet akagent build-docker build-package

.PHONY: akfs akps akfile aknet akagent build-docker build-package

build-package:
	test -d $(packageDir) || mkdir -p $(packageDir)
	install -p -D -m 0755 ./akfs/akfs.ko $(packageDir)
	install -p -D -m 0755 ./akps/process.ko $(packageDir)
	install -p -D -m 0755 ./aknet/net.ko $(packageDir)
	install -p -D -m 0755 ./akfile/file.ko $(packageDir)
	
	install -p -D -m 0755 ./akagent/akagent $(packageDir)
	install -p -D -m 0755 ./akagent/*.json $(packageDir)
	install -p -D -m 0755 ./akagent/*.filter $(packageDir)
	install -p -D -m 0755 $(akagentDockerPkg) $(packageDir)

	install -p -D -m 0755 install.sh $(packageDir)
	tar -czf  $(packageDir).tar.gz  $(packageDir)
	echo -e "\033[32m  build package. new package is $(packageDir).tar.gz \033[0m"

build-docker:
	docker build -t bxsec/akagent:$(version) ./akagent/.
	docker save  -o  $(akagentDockerPkg)   bxsec/akagent:$(version)

akfs:
	cd akfs/ && ./configure && make
	echo -e "\033[32m build akfs done \033[0m"


akps:
	cd akps/ && ./configure && make
	echo -e "\033[32m build akps done \033[0m"
akfile:
	cd akfile/ && ./configure && make
	echo -e "\033[32m build akfile done \033[0m"
aknet:
	cd aknet/ && ./configure && make
	echo -e "\033[32m build aknet done \033[0m"


akagent:
	make -C ./akagent/
	echo -e "\033[32m build akagent done \033[0m"
