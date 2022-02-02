FROM alpine

RUN apk add --no-cache gcc cmake make musl-dev linux-headers

COPY . /rhasheq

WORKDIR /rhasheq/build

RUN cmake -DCMAKE_BUILD_TYPE="Debug" .. && \
    cmake --build .

CMD ctest .
