--- hw1/dev_repo/basecode/hw1/include/bdd.h

+++ hw1/repos/xinchi/hw1/include/bdd.h

@@ -198,23 +198,18 @@

 

 /**

  * Given a BDD node that represents a 2^d x 2^d image, construct a new

- * BDD node that represents the result of "zooming" by a specified

- * nonnegative "factor" k.  "Zooming in" is an operation that increases

- * the number of pixels by replacing each pixel in the original image

+ * BDD node that represents the result of "zooming in" by a specified

+ * nonnegative "factor" k.  Zooming in is an operation that increases

+ * the number of pixels by replacing each pixel in the individual image

  * by a 2^k x 2^k array of identical pixels in the transformed image.

- * "Zooming out" is an operation that decreases the number of pixels

- * by replacing each 4x4 square of pixels in the original image by a

- * single pixel.  As discussed in the assignment handout, this single

- * pixel should be a black pixel if the 4x4 square is entirely black,

- * otherwise it is a white pixel.  Zooming by a factor of 0 is an

- * identity transformation.

+ * Zooming by a factor of 0 is an identity transformation.

  *

  * @param node  The BDD node to transform.

  * @param level  The level at which to interpret the node,

  * which (due to "skipped levels") might be larger than the level

  * recorded in the node itself.

- * @param factor   The "zoom factor", which must be a value

- * satisfying -BDD_LEVELS_MAX <= d+k <= BDD_LEVELS_MAX.

+ * @param factor   The "zoom factor", which must be a nonnegative value

+ * satisfying 0 <= d+k <= BDD_LEVELS_MAX.

  * @return  the BDD node that represents the result of the "zoom"

  * operation.

  */
