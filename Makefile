gui:
	mkdir -p ./build/gui-Release
	cd ./build/gui-Release && \
	qmake /home/lrq/Desktop/VehicleSystem/src/GUI/GUI.pro -spec devices/linux-buildroot-g++ \
	CONFIG+=object_files_dir=. \
	CONFIG+=moc_files_dir=. \
	CONFIG+=rcc_files_dir=. \
	CONFIG+=ui_files_dir=. && \
	make qmake_all && \
	bear -- make -s -j4
	cp ./build/gui-Release/compile_commands.json ./src/GUI/

clean:
	rm -rf ./build/gui-Release

install: gui
	cp -r ./build/gui-Release/GUI /home/lrq/linux/nfs/qtrootfs/qt_apps