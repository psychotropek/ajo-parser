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
		
		$source_config = $source->getConfigFor($this);
		$state = $source->state(FEEDS_PARSE);


		/*$files = $fetcher_result->getFilePath();	
		var_dump($result);
		die;
		//return $files;
		
		//foreach file, get the extension
		foreach ($files as $file) {
			$item = array();
			//one item should contain an array with the filename, the 
			$path_parts = pathinfo($file); //check for result
			$filetype = $path_parts['extension'];
			switch($status) {
				case "doc":
				case "docx":
					//$item['html'] = getRaw, then...parse?

/*






				break;
				case "xls":
				case "xslx":
					//get suffix of file for number (figure1, figure2, etc)
					//for excel files, getRaw, then call the API. Pass the html into an array. 
				break;
				case "png":
				case "jpg":
				case "jpeg":
					//get suffix of file for number, same as xls (figure1, figure2, figure3, small image, large image, etc)
					//small image or large image, handle
					//otherwise for image files, get some metadata (exif? or do we need to get raw?) for caption

				break;
					//todo: otherwise, move on?	

	//here, parse through things and add them to the result items array
	//$item = array('url' => (string) $url->loc);

				//$result->items[] = $item;
			}


			
		}*/

		return $result;
	}

	/**
	 * Override parent::getMappingSources().
	 * n.b. this is false b/c we're hardcoding all of this
	 */
	public function getMappingSources() {
		return FALSE;
	}



	/**
	 * Define default configuration.
	 * n.b. todo: removing this really messes things up
	 */
	public function configDefaults() {
		$config = parent::configDefaults();
	}

	//todo: there may be something missing here?
}
	