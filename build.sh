mkdir build
cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target STC_TaskTest
read -p "Press enter to continue"