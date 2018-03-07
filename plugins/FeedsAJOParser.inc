<?php

/**
 * @file
 * Contains the FeedsAJOParser class.
 */

class FeedsAJOParser extends FeedsParser {

	/**
	* Implements FeedsParser::parse().
	*/

	public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {
		$result = new FeedsParserResult();
		$files=$fetcher_result->file_list;

//array(2) { [0]=> string(57) "public://articles/ajo_test_2/testzip_9/amp_um_flower.jpeg" 
	//[1]=> string(63) "public://articles/ajo_test_2/testzip_9/book reviews part 2.docx" }

		//foreach file, get the extension
		foreach ($files as $file) {
			$item = array();
			//one item should contain an array with the filename, the 
			$path_parts = pathinfo($file); //check for result
			$file_type = $path_parts['extension'];
			switch($file_type) {
				case "doc":
				case "docx":
					//$item['html'] = getRaw, then...parse?


				break;
				case "xls":
				case "xslx":
					//get suffix of file for number (figure1, figure2, etc)
					//for excel files, getRaw, then call the API. Pass the html into an array. 
				break;
				case "png":
				case "jpg":
				case "jpeg":
					//todo: file is just a string.
					//get suffix of file for number, same as xls (figure1, figure2, figure3, small image, large image, etc)
					//small image or large image, handle
					//otherwise for image files, get some metadata (exif? or do we need to get raw?) for caption
					$item['caption'] = exif_read_data($file, 'exif');
var_dump($item['caption']);
				break;
					//todo: otherwise, move on?	

				$result->items[] = $item;
			}


			
		}

		return $result;
	}

}
	