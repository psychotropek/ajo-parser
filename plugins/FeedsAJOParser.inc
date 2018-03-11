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

		require DRUPAL_ROOT.'/sites/all/vendor/autoload.php';

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

//todo: try if
					try {
						$client = new GuzzleHttp\Client(['base_uri' => 'https://api.docconverter.pro','verify' => false]);
						$res = $client->post('/Token', [ 'form_params' => [ 'grant_type' => 'password', 'username' => 'chris@quillandcode.com', 'password' => 'AJO_d3v070p'] ]);
						$statusCode = $res->getStatusCode();
						if ( $statusCode != 200 )
						{
							echo 'Invalid request status code: ' . $statusCode;
							die();
						}
						$body = (string)$res->getBody();
						$data = json_decode($body);
						//echo 'Token: ' . $data->access_token;
						$token = $data->access_token;
						// $token can be stored in site cache or database, please check expiry date ($data->expires_in) before using token again (2 weeks expiration time)

						$formData = array();
						//$formData[] = [ 'name' => 'file_name', 'contents' => file_get_contents($file)];
						//Original command. Trying something else?
						$formData[] = [ 'name' => 'file_name', 'contents' => fopen($file, 'r')];
						
						$formData[] = [ 'name' => 'template', 'contents' => 'AJO test template'];
						$formData[] = [ 'name' => 'returnHtml', 'contents' => 'true'];
						$formData[] = [ 'name' => 'returnData', 'contents' => 'true'];

						
						$response = $client->request('POST', '/api/converter/convertdoc', [
							'headers' => [ 'Authorization' => 'Bearer ' . $token ],
							'multipart' => $formData,
						]);
                    	
						$htmldoc = $response->getBody();
						
					}
					catch (Exception $exc) {
						echo $exc->getMessage();
					}

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
					//todo: ask Chris what exif data it will be kept in- send a sample image
				break;
					//todo: otherwise, move on?	

				$result->items[] = $item;
			}


			
		}

		return $result;
	}

}
	