#
# Created by loumouli on 12/15/23.
#

all:
	cmake -S . -B cmake-build-debug
	cmake --build cmake-build-debug -j 12

clean:
	find ./cmake-build-debug -type f ! -name 'ft_ping' -delete
	find ./cmake-build-debug -type d ! -path './cmake-build-debug' -exec rm -rf {} +

fclean:
	rm -rf ./cmake-build-debug

re:			fclean all

.PHONY: all clean fclean re

.NOTPARALLEL: fclean